/*   extasy - Extensible Tracker And Synthesizer
 *   Copyright (C) 2014 Stefan T. Boettner
 *
 *   extasy is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   extasy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with extasy.  If not, see <http://www.gnu.org/licenses/>. */
 
#include <algorithm>
#include <stdio.h>
#include <math.h>
#include <queue>
#include <fftw3.h>
#include <RtMidi.h>
#include <gui/gui.h>
#include <gui/filechooser.form.h>
#include <synth/synthbits.h>
#include <synth/module.h>
#include <synth/sequencer.h>
#include <synth/padsynth.h>
#include <synth/fmsynth.h>
#include <synth/additivesynth.h>
#include <synth/pitchless.h>
#include <synth/pwmsynth.h>
#include <synth/hypersaw.h>
#include <synth/kickdrum.h>
#include <synth/customsynth.h>
#include <synth/serialization.h>
#include <synthedit/synthedit.h>
#include <synthedit/baseinstredit.form.h>
#include "patternedit.h"
#include "arrangementedit.h"

using namespace GUI;
using namespace Synth;

Mixer* mixer;

unsigned int glsl_load_shader(int type, const char* filename, const char* preamble=0)
{
	FILE* file=fopen(filename, "r");
	if (!file) {
		fprintf(stderr, "Error opening %s\n", filename);
		abort();
	}
	
	fseek(file, 0, SEEK_END);
	int size=ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* buffer=new char[size+1];
	fread(buffer, 1, size, file);
	fclose(file);

	buffer[size]=0;
	
	const char* src[]={ preamble?preamble:"", buffer };
	
	unsigned int name=glCreateShader(type);
	glShaderSource(name, 2, (const GLchar**) src, 0);
	delete[] buffer;
	
	glCompileShader(name);
	
	int foo;
	glGetShaderiv(name, GL_COMPILE_STATUS, &foo);
	if (foo==GL_TRUE) return name;
	
	char msg[1024];
	glGetShaderInfoLog(name, 1024, &foo, msg);
	fprintf(stderr, "Error compiling glsl program '%s':\n%s\n", filename, msg);
	
	exit(1);
}

unsigned int glsl_link_program(unsigned int shv, unsigned int shf)
{
	unsigned int sh=glCreateProgram();

	glAttachShader(sh, shv);
	glAttachShader(sh, shf);
	glLinkProgram(sh);

	return sh;
}

unsigned int glsl_link_program(unsigned int shv, unsigned int shg, unsigned int shf, int vert_out, int geom_in, int geom_out)
{
	unsigned int sh=glCreateProgram();

	glAttachShader(sh, shv);
	glAttachShader(sh, shg);
	glAttachShader(sh, shf);
	
	glProgramParameteriEXT(sh, GL_GEOMETRY_VERTICES_OUT_EXT, vert_out);
	glProgramParameteriEXT(sh, GL_GEOMETRY_INPUT_TYPE_EXT, geom_in);
	glProgramParameteriEXT(sh, GL_GEOMETRY_OUTPUT_TYPE_EXT, geom_out);
	
	glLinkProgram(sh);

	int foo;
	glGetProgramiv(sh, GL_LINK_STATUS, &foo);
	if (foo==GL_TRUE) return sh;
	
	char msg[1024];
	glGetProgramInfoLog(sh, 1024, &foo, msg);
	fprintf(stderr, "Error linking glsl program:\n%s\n", msg);
	//return sh;
	exit(1);
}

struct AudioBlock {
	unsigned char*	data;
	int		size;
	int		consumed;
	
	explicit AudioBlock(int s)
	{
		size=s;
		consumed=0;
		data=new unsigned char[size];
	}
	
	~AudioBlock()
	{
		delete[] data;
	}
};

std::queue<AudioBlock*> audio_queue;

void audio_callback(void* userdata, unsigned char* stream, int length)
{
	int i;
	
	while (length>0) {
		if (audio_queue.empty()) {
			for (i=0;i<length;i++)
				stream[i]=0;
			break;
		}
	
		AudioBlock* blk=audio_queue.front();
		while (blk->consumed<blk->size && length-->0)
			*stream++=blk->data[blk->consumed++];
		
		if (blk->consumed==blk->size) {
			audio_queue.pop();
			delete blk;
		}
	}
	
	SDL_Event event;
	event.type=SDL_AUDIO_CONSUMED;
	SDL_PushEvent(&event);
}

void init_audio()
{
	SDL_AudioSpec spec;
	
	spec.freq=48000;
	spec.format=AUDIO_S16SYS;
	spec.channels=2;
	spec.samples=512;
	spec.callback=audio_callback;
	spec.userdata=0;

	if (SDL_OpenAudio(&spec, NULL) < 0) {
		fprintf(stderr, "Error opening audio device:\n%s\n", SDL_GetError());
		exit(1);
	}
}

void midicallback(double deltatime, std::vector<unsigned char>* message, void *userdata)
{
	SDL_Event event;
	
	unsigned char* data=new unsigned char[message->size()];
	for (int i=0;i<message->size();i++)
		data[i]=(*message)[i];
	
	event.type=SDL_MIDI_EVENT;
	event.user.code=message->size();
	event.user.data1=data;
	event.user.data2=nullptr;
	
	SDL_PushEvent(&event);
}

template<typename T>
static inline T clamp(T val, T low, T high)
{
	if (val<low)
		return low;
	else if (val>high)
		return high;
	else
		return val;
}

template<typename T>
static inline T sqr(T x)
{
	return x*x;
}

template<typename T>
static inline T cube(T x)
{
	return x*x*x;
}

class SpectrumAnalyzer:public Widget {
public:
	SpectrumAnalyzer(int, int);
	virtual ~SpectrumAnalyzer();

	virtual void draw();
	
	void feed(float**, int);
	
private:
	static int	shader;
	
	int			resolution;
	int			historylength;
	int			fill_ptr;
	int			fill_row;
	
	unsigned int	texture;
	
	float*		databuffer;
	float*		window;
	
	double*		time_domain;
	double*		freq_domain;
	fftw_plan	fft;
	
};

int SpectrumAnalyzer::shader=0;

SpectrumAnalyzer::SpectrumAnalyzer(int res, int histlen):Widget(0,0,res,histlen), resolution(res), historylength(histlen)
{
	if (!shader) {
		int shv=glsl_load_shader(GL_VERTEX_SHADER, "shader/spectrum.vert");
		int shf=glsl_load_shader(GL_FRAGMENT_SHADER, "shader/spectrum.frag");
		shader=glsl_link_program(shv, shf);
	}
	
	window=new float[2*resolution];
	databuffer=new float[2*resolution];
	time_domain=new double[2*resolution];
	freq_domain=new double[2*resolution];
	
	fft=fftw_plan_r2r_1d(2*resolution, time_domain, freq_domain, FFTW_R2HC, FFTW_ESTIMATE);
	
	for (int i=0;i<2*resolution;i++)
		window[i]=sqr(sinf(M_PI*(2*i+1)/(4*resolution)));
	
	fill_ptr=0;
	fill_row=0;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	float* allzero=new float[resolution*historylength];
	for (int i=0;i<resolution*historylength;i++)
		allzero[i]=0.0f;
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, resolution, historylength, 0, GL_RED, GL_FLOAT, allzero);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	delete[] allzero;
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
	glDeleteTextures(1, &texture);
	
	fftw_destroy_plan(fft);
	
	delete[] window;
	delete[] time_domain;
	delete[] freq_domain;
}

void SpectrumAnalyzer::feed(float** samples, int count)
{
	int i,j;
	
	for (i=0;i<count;i++) {
		databuffer[fill_ptr++]=samples[0][i] + samples[1][i];
		
		if (fill_ptr==2*resolution) {
			fill_ptr=resolution;

			for (j=0;j<resolution;j++)
				time_domain[j]=databuffer[j]*window[j];
			for (j=resolution;j<2*resolution;j++) {
				time_domain[j]=databuffer[j]*window[j];
				databuffer[j-resolution]=databuffer[j];
			}
			
			fftw_execute(fft);
			
			float rowdata[resolution];
			
			rowdata[0]=0;
			for (j=1;j<resolution;j++)
				rowdata[j]=sqr(freq_domain[j]) + sqr(freq_domain[2*resolution-j]);
			
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, fill_row++, resolution, 1, GL_LUMINANCE, GL_FLOAT, rowdata);
			if (fill_row==historylength) fill_row=0;
		}
	}
}

void SpectrumAnalyzer::draw()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "spectrum"), 0);
	glUniform1f(glGetUniformLocation(shader, "currentrow"), (float) fill_row/historylength);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2i(originx, originy);
	glTexCoord2f(1, 0);
	glVertex2i(originx+width, originy);
	glTexCoord2f(1, 1);
	glVertex2i(originx+width, originy+height);
	glTexCoord2f(0, 1);
	glVertex2i(originx, originy+height);
	glEnd();

	glUseProgram(0);
}

class PeakAnalyzer:public LightBar {
public:
	PeakAnalyzer(int, int, int, int);
	virtual ~PeakAnalyzer();

	void feed(float*, int);
	
private:
	float	peaks[4];
	int		ptr;
};

PeakAnalyzer::PeakAnalyzer(int x, int y, int w, int h)
{
	set_origin(x, y);
	set_size(w, h);
	
	peaks[0]=peaks[1]=peaks[2]=peaks[3]=0;
	ptr=0;
}

PeakAnalyzer::~PeakAnalyzer()
{
}

void PeakAnalyzer::feed(float* data, int count)
{
	float peak=0;
	for (int i=0;i<count;i++)
		peak=std::max(peak, fabsf(data[i]));
		
	peaks[ptr++]=peak;
	ptr&=3;
	
	set_value(log10f((peaks[0]+peaks[1]+peaks[2]+peaks[3])/4)*0.25f+1.0f);
}

void quit_clicked()
{
	SDL_Event ev;
	ev.type=SDL_QUIT;
	SDL_PushEvent(&ev);
}

class NewInstrumentDlg:public Window {
	Module*		module;
	int			instrument_number;
	ListBox*	class_list_box;
	
	class ClassNameColumn:public ListBox::Column {
		virtual void draw(int, int, int, bool) const;
		
	public:
		ClassNameColumn():Column(368) {}
	};
	
	void create();
	void load();
	
public:
	NewInstrumentDlg(Module*, int);
};

NewInstrumentDlg::NewInstrumentDlg(Module* mod, int instrno):Window(256, 256, 512, 256), module(mod), instrument_number(instrno)
{
	set_title("New Instrument");
	set_modal(true);
	
	class_list_box=new ListBox(8, 32, 368, 216);
	class_list_box->add_column(new ClassNameColumn());
	class_list_box->set_entry_count(Instrument::get_class_count());
	add(class_list_box);
	
	Button* but=new Button(392, 32, 112, 24, "Create");
	but->clicked.connect(sigc::mem_fun(this, &NewInstrumentDlg::create));
	add(but);
	
	but=new Button(392, 64, 112, 24, "Load...");
	but->clicked.connect(sigc::mem_fun(this, &NewInstrumentDlg::load));
	add(but);
	
	but=new Button(392, 224, 112, 24, "Cancel");
	but->clicked.connect(sigc::mem_fun(this, &Window::close));
	add(but);
}

void NewInstrumentDlg::create()
{
	close();
	
	module->instruments[instrument_number]=Instrument::create(*mixer, Instrument::get_class_name(class_list_box->get_selection()));
	
	// FIXME
	//new_instrument_button->set_visible(false);
	//edit_instrument_button->set_visible(true);
}

void NewInstrumentDlg::load()
{
	FileChooserDialog* filechooser=FileChooserDialog::create_load_dialog("Load Instrument from File...");
	if (filechooser->run(get_root_window())) {
		delete filechooser;
		return;
	}
	
	close();
	
	FileDeserializer deser(filechooser->get_filename().c_str(), *mixer);
	delete filechooser;
	
	Instrument* instr;
	
	int magic, version;
	deser >> magic >> tag("version", version);
	// FIXME: check magic and version
	
	deser >> instr;
	
	module->instruments[instrument_number]=instr;
}

void NewInstrumentDlg::ClassNameColumn::draw(int x, int y, int row, bool highlighted) const
{
	float c=highlighted ? 1.0f : 0.7f;
	
	textprint(x, y, c, c, c, Instrument::get_class_name(row));
}

class InstrumentNameColumn:public ListBox::Column {
	ListBox*	listbox;
	Module*&	module;
	
	virtual void draw(int, int, int, bool) const;
	virtual Widget* edit(int);
		
	void accept_edit(LineEdit*, int);

public:
	InstrumentNameColumn(ListBox* lb, Module*& mod):Column(256), listbox(lb), module(mod) {}
};

void InstrumentNameColumn::draw(int x, int y, int row, bool highlighted) const
{
	if (!module) return;
	Instrument* instr=module->instruments[row];
	
	FontSpec font;
	font.color=instr ? instr->get_color() : 0x808080;
	font.color=font.color.shade(highlighted ? 160 : 96);
	
	if (instr)
		textprint(x, y, font, instr->get_name());
	else
		textprint(x, y, font, "No instrument");
}

Widget* InstrumentNameColumn::edit(int row)
{
	if (!module) return nullptr;
	
	Instrument* instr=module->instruments[row];
	if (!instr) return nullptr;
	
	LineEdit* ed=new LineEdit(0, 0, width-2, 24);
	ed->set_text(instr->get_name());
	ed->activate_cursor();
	
	ed->finished.connect(sigc::hide(sigc::bind(sigc::mem_fun(this, &InstrumentNameColumn::accept_edit), ed, row)));
	ed->lost_input_focus.connect(sigc::hide(sigc::bind(sigc::mem_fun(this, &InstrumentNameColumn::accept_edit), ed, row)));
	
	ed->finished.connect(sigc::hide(sigc::mem_fun(listbox, &ListBox::edit_finished)));
	ed->cancelled.connect(sigc::hide(sigc::mem_fun(listbox, &ListBox::edit_cancelled)));
	ed->lost_input_focus.connect(sigc::hide(sigc::mem_fun(listbox, &ListBox::edit_finished)));
	
	return ed;
}

void InstrumentNameColumn::accept_edit(LineEdit* ed, int row)
{
	if (!module) return;
	
	Instrument* instr=module->instruments[row];
	
	if (instr)
			instr->set_name(ed->get_text());
}

class PatternNameColumn:public ListBox::Column {
	ListBox*	listbox;
	Module*&	module;
	
	virtual void draw(int, int, int, bool) const;
	virtual Widget* edit(int);
		
	void accept_edit(LineEdit*, int);

public:
	PatternNameColumn(ListBox* lb, Module*& mod):Column(256), listbox(lb), module(mod) {}
};

void PatternNameColumn::draw(int x, int y, int row, bool highlighted) const
{
	if (!module) return;
	
	Pattern* pattern=module->patterns[row];
	
	FontSpec font;
	font.color=0xc0c0c0;
	font.color=font.color.shade(highlighted ? 160 : 96);
	
	textprint(x, y, font, pattern->get_name());
}

Widget* PatternNameColumn::edit(int row)
{
	if (!module) return nullptr;
	
	Pattern* pattern=module->patterns[row];
	
	LineEdit* ed=new LineEdit(0, 0, width-2, 24);
	ed->set_text(pattern->get_name());
	ed->activate_cursor();
	
	ed->finished.connect(sigc::hide(sigc::bind(sigc::mem_fun(this, &PatternNameColumn::accept_edit), ed, row)));
	ed->lost_input_focus.connect(sigc::hide(sigc::bind(sigc::mem_fun(this, &PatternNameColumn::accept_edit), ed, row)));
	
	ed->finished.connect(sigc::hide(sigc::mem_fun(listbox, &ListBox::edit_finished)));
	ed->cancelled.connect(sigc::hide(sigc::mem_fun(listbox, &ListBox::edit_cancelled)));
	ed->lost_input_focus.connect(sigc::hide(sigc::mem_fun(listbox, &ListBox::edit_finished)));
	
	return ed;
}

void PatternNameColumn::accept_edit(LineEdit* ed, int row)
{
	if (!module) return;
	
	Pattern* pattern=module->patterns[row];
	pattern->set_name(ed->get_text());
}

class TrackerApp:public MainWindow {
	PatternEdit* pattern_edit;
	ListBox* pattern_list;
	
	ListBox* instrument_list;
	Button* new_instrument_button;
	Button* edit_instrument_button;

	SpinBox* bpm_box;
	PeakAnalyzer *pa_left, *pa_right;
	SpectrumAnalyzer* spectrum_analyzer;
	ArrangementEdit* arrangement_edit;
	PatternStockEdit* pattern_stock_edit;

	RadioButtonGroup colorgroup;
	ColorLabelButton* colorlabelbutton;

	Mixer* mixer;
	Sequencer* sequencer;
	Module*	module;
	
	bool play;
	int tick_samples;
	
	
	virtual void handle_event(SDL_Event&);
	
	void instrument_selection_changed(int sel);
	void edit_instrument();
	void create_new_instrument();
	void pattern_name_edited(int index, const char* name);
	void select_pattern_by_index(int index);
	void select_pattern(Pattern*);
	void save_instr_clicked();
	void save_module_clicked();
	void load_module_clicked();
	void new_module_clicked();
	void toggle_spectrum_analyzer();
	void toggle_arrangement_editor();
	void toggle_pattern_editor();

public:	
	TrackerApp();

	void set_mixer(Mixer*);
	void set_module(Module*);
};

TrackerApp::TrackerApp()
{
	play=false;
	tick_samples=0;
	
	ScrollPane* pattern_edit_pane=new ScrollPane(true, true);
	pattern_edit_pane->set_size(get_width(), get_height()-128);
	pattern_edit_pane->set_origin(0, 128);
	add(pattern_edit_pane);
	
	pattern_edit=new PatternEdit;
	pattern_edit->set_origin(0, 128);
	pattern_edit->set_size((16*15+4)*8, 65*16);
	pattern_edit_pane->add(pattern_edit);
	
	HPaned* paned=new HPaned(0, 0, get_width(), 128);
	add(paned);

	instrument_list=new ListBox(0,0,256,128);
	instrument_list->add_column(new ListBox::IndexColumn());
	instrument_list->add_column(new InstrumentNameColumn(instrument_list, module));
	instrument_list->set_entry_count(256);
	instrument_list->selection_changed.connect(sigc::mem_fun(this, &TrackerApp::instrument_selection_changed));
	paned->add_pane("Instruments", instrument_list);
	
	Group* pane=new Group(0, 0, 128, 128);
	paned->add_pane("Actions", pane);
	
	Button* but=new_instrument_button=new Button(0, 8, 128, 24, "New Instrument");
	but->clicked.connect(sigc::mem_fun(this, &TrackerApp::create_new_instrument));
	pane->add(but);

	but=edit_instrument_button=new Button(0, 8, 128, 24, "Edit Instrument");
	but->clicked.connect(sigc::mem_fun(this, &TrackerApp::edit_instrument));
	pane->add(but);
	
	but=new Button(0, 40, 128, 24, "Save Instrument");
	but->clicked.connect(sigc::mem_fun(this, &TrackerApp::save_instr_clicked));
	pane->add(but);
	
	but=new Button(0, 72, 128, 24, "Quit");
	but->clicked.connect(sigc::ptr_fun(quit_clicked));
	pane->add(but);
	
	pattern_list=new ListBox(512, 0, 512, 128);
	pattern_list->add_column(new PatternNameColumn(pattern_list, module));
	pattern_list->selection_changed.connect(sigc::mem_fun(this, &TrackerApp::select_pattern_by_index));
	pattern_list->entry_edited.connect(sigc::mem_fun(this, &TrackerApp::pattern_name_edited));
	paned->add_pane("Patterns", pattern_list);
	
	pane=new Group(0, 0, 128, 128);
	pa_left=new PeakAnalyzer(0, 2, 128, 16);
	pa_right=new PeakAnalyzer(0, 22, 128, 16);
	pane->add(pa_left);
	pane->add(pa_right);
	
	bpm_box=new SpinBox(16, 64, 96, 24);
	pane->add(bpm_box);
	paned->add_pane("Peak Meter", pane);
	
	Knob* knob=new Knob(128, 48, 48, 64);
	knob->set_label("Volume");
	knob->set_midi_controller(7);
	knob->set_position(1.0f);
	knob->set_range(0, 1);
	knob->value_changed.connect([this](float pos, float vol) { mixer->set_master_volume(vol); });
	//knob->value_changed.connect(sigc::hide<0>(sigc::mem_fun(mixer, &Mixer::set_master_volume)));
	pane->add(knob);
	
	knob=new Knob(192, 48, 48, 64);
	knob->set_label("Modulation");
	knob->set_midi_controller(1);
	knob->set_position(0.5f);
	knob->set_range(0, 1);
	knob->value_changed.connect(sigc::hide<0>(sigc::mem_fun(pattern_edit, &PatternEdit::set_modulation)));
	pane->add(knob);
	
	but=new Button(256, 16, 128, 24, "Spectrum Analyzer");
	but->clicked.connect(sigc::mem_fun(this, &TrackerApp::toggle_spectrum_analyzer));
	pane->add(but);
	
	but=new Button(256, 48, 128, 24, "Arrangement Editor");
	but->clicked.connect(sigc::mem_fun(this, &TrackerApp::toggle_arrangement_editor));
	pane->add(but);
	
	but=new Button(256, 80, 128, 24, "Pattern List");
	but->clicked.connect(sigc::mem_fun(this, &TrackerApp::toggle_pattern_editor));
	pane->add(but);
	
	but=new Button(400, 16, 128, 24, "New Module");
	but->clicked.connect(sigc::mem_fun(this, &TrackerApp::new_module_clicked));
	pane->add(but);
	
	but=new Button(400, 48, 128, 24, "Load Module");
	but->clicked.connect(sigc::mem_fun(this, &TrackerApp::load_module_clicked));
	pane->add(but);
	
	but=new Button(400, 80, 128, 24, "Save Module");
	but->clicked.connect(sigc::mem_fun(this, &TrackerApp::save_module_clicked));
	pane->add(but);

	for (int i=0;i<6;i++) {
		LightBar* lightbar=new LightBar;
		lightbar->set_size(128, 16);
		lightbar->set_origin(544, 12+i*16);
		lightbar->set_value((i+3)*0.125);
		pane->add(lightbar);
	}
	
	spectrum_analyzer=nullptr;
	arrangement_edit=nullptr;
	pattern_stock_edit=nullptr;
}

void TrackerApp::set_mixer(Mixer* m)
{
	mixer=m;
	
	sequencer=new Sequencer(*m);
}

void TrackerApp::set_module(Module* m)
{
	if (pattern_stock_edit)
		toggle_pattern_editor();
	if (arrangement_edit)
		toggle_arrangement_editor();

	module=m;
	
	sequencer->set_module(module);
	
	pattern_list->set_entry_count(module->patterns.size());
	
	pattern_edit->set_module(module);
	pattern_edit->set_pattern(module->patterns[0]);
}

void TrackerApp::handle_event(SDL_Event& event)
{
	MainWindow::handle_event(event);
	
	switch(event.type) {
	case SDL_KEYDOWN:
		if (event.key.keysym.sym==SDLK_TAB) {
			play=!play;
			mixer->stop_all_tones();
			sequencer->reset_position();
		}
		break;
	}
		
	while (audio_queue.size() < 3) {
		int blksize=1024;
		AudioBlock* blk=new AudioBlock(blksize*4);
		float* data[2];
		
		data[0]=new float[blksize];
		data[1]=new float[blksize];
		
		int ptr=0;
		while (ptr<blksize) {
			if (!tick_samples) {
				if (play) {
					send_notification(SDL_PLAY_POSITION_NOTIFICATION, (void*) sequencer->get_current_arrangement_item(), sequencer->get_current_row());
					sequencer->process_row();
				}
				else
					send_notification(SDL_PLAY_POSITION_NOTIFICATION, nullptr, -1);
					
				tick_samples=mixer->get_samplerate() * 15 / bpm_box->get_value();
			}
			
			int fill=std::min(tick_samples, blksize-ptr);
			
			float* tmp[2]={ data[0]+ptr, data[1]+ptr };
			mixer->mix(tmp, fill);
			
			ptr+=fill;
			tick_samples-=fill;
		}
		
		for (int i=0;i<blksize;i++) {
			reinterpret_cast<short*>(blk->data)[2*i  ]=lrint(32767*clamp(data[0][i], -1.0f, 1.0f));
			reinterpret_cast<short*>(blk->data)[2*i+1]=lrint(32767*clamp(data[1][i], -1.0f, 1.0f));
		}
		
		if (spectrum_analyzer)
			spectrum_analyzer->feed(data, blksize);
			
		pa_left->feed(data[0], blksize);
		pa_right->feed(data[1], blksize);
		
		delete[] data[0];
		delete[] data[1];
		
		audio_queue.push(blk);
	}
}

void TrackerApp::instrument_selection_changed(int sel)
{
	new_instrument_button ->set_visible(module->instruments[sel]==NULL);
	edit_instrument_button->set_visible(module->instruments[sel]!=NULL);
	
	pattern_edit->set_instrument(sel);
}

void TrackerApp::create_new_instrument()
{
	NewInstrumentDlg* dlg=new NewInstrumentDlg(module, instrument_list->get_selection());
	
	add(dlg);
	bring_to_front(dlg);
}

void TrackerApp::edit_instrument()
{
	Instrument* instr=module->instruments[instrument_list->get_selection()];
	if (!instr) return;
	
	/*Window* wnd=Synth::IInstrumentEditorProvider::create(instr);
	if (!wnd) return;*/
	
	Window* wnd=new Synth::BasicInstrumentEdit(instr);
	wnd->show(this);
}

void TrackerApp::pattern_name_edited(int index, const char* name)
{
	module->patterns[index]->set_name(name);
}

void TrackerApp::select_pattern_by_index(int index)
{
	select_pattern(module->patterns[index]);
}

void TrackerApp::select_pattern(Pattern* pattern)
{
	pattern_edit->set_pattern(pattern);
	pattern_list->set_selection(pattern->get_index());
	
	if (arrangement_edit)
		dispatch(sigc::mem_fun(this, &TrackerApp::toggle_arrangement_editor));
		
	if (pattern_stock_edit)
		dispatch(sigc::mem_fun(this, &TrackerApp::toggle_pattern_editor));
}

void TrackerApp::save_instr_clicked()
{
	Instrument* instr=module->instruments[instrument_list->get_selection()];
	if (!instr) return;
	
	FileChooserDialog* filechooser=FileChooserDialog::create_save_dialog("Save Instrument to File...");
	if (filechooser->run(this)) {
		delete filechooser;
		return;
	}
	
	Serializer ser;
	ser << INSTRUMENT_FILE_MAGIC;

	int version=1;
	ser << tag("version", version);
	
	ser << instr;
	ser.write_to_file(filechooser->get_filename().c_str());
	
	delete filechooser;
}

void TrackerApp::save_module_clicked()
{
	FileChooserDialog* filechooser=FileChooserDialog::create_save_dialog("Save Module...");
	if (filechooser->run(this)) {
		delete filechooser;
		return;
	}
	
	Serializer ser;
	ser << MODULE_FILE_MAGIC;

	int version=1;
	ser << tag("version", version);
	
	ser << module;
	ser.write_to_file(filechooser->get_filename().c_str());
	
	delete filechooser;
}

void TrackerApp::load_module_clicked()
{
	FileChooserDialog* filechooser=FileChooserDialog::create_load_dialog("Load Module...");
	if (filechooser->run(get_root_window())) {
		delete filechooser;
		return;
	}
	
	FileDeserializer deser(filechooser->get_filename().c_str(), *mixer);
	delete filechooser;
	
	Instrument* instr;
	
	int magic, version;
	deser >> magic >> tag("version", version);
	// FIXME: check magic and version
	
	Module* mod;
	deser >> mod;
	
	if (mod) {
		delete module;
		set_module(mod);
	}
}

void TrackerApp::new_module_clicked()
{
	delete module;
	set_module(new Module);
}

void TrackerApp::toggle_spectrum_analyzer()
{
	if (spectrum_analyzer) {
		remove(spectrum_analyzer);
		delete spectrum_analyzer;
		spectrum_analyzer=nullptr;
		
		pattern_edit->set_size(get_width(), get_height()-128);
	}
	else {
		spectrum_analyzer=new SpectrumAnalyzer(get_width(), 256);
		spectrum_analyzer->set_origin(0, get_height()-256);
		add(spectrum_analyzer);
		bring_to_front(spectrum_analyzer);
		
		pattern_edit->set_size(get_width(), get_height()-384);
	}
}

void TrackerApp::toggle_arrangement_editor()
{
	if (pattern_stock_edit)
		toggle_pattern_editor();
		
	if (arrangement_edit) {
		remove(arrangement_edit);
		delete arrangement_edit;
		arrangement_edit=nullptr;
	}
	else {
		arrangement_edit=new ArrangementEdit;
		arrangement_edit->set_module(module);
		arrangement_edit->set_origin(0, 128);
		arrangement_edit->set_size(get_width(), get_height()-128);
		arrangement_edit->select_pattern.connect(sigc::mem_fun(this, &TrackerApp::select_pattern));
		add(arrangement_edit);
	}
}

void TrackerApp::toggle_pattern_editor()
{
	if (arrangement_edit)
		toggle_arrangement_editor();
		
	if (pattern_stock_edit) {
		remove(pattern_stock_edit);
		delete pattern_stock_edit;
		pattern_stock_edit=nullptr;
	}
	else {
		pattern_stock_edit=new PatternStockEdit;
		pattern_stock_edit->set_module(module);
		pattern_stock_edit->set_origin(0, 128);
		pattern_stock_edit->set_size(get_width(), get_height()-128);
		pattern_stock_edit->select_pattern.connect(sigc::mem_fun(this, &TrackerApp::select_pattern));
		add(pattern_stock_edit);
	}
}

int main(int argn, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	
	TrackerApp* desktop=new TrackerApp();
	
	srand(time(0));
	
	try {
		RtMidiIn* midi=new RtMidiIn();
		midi->openPort(1);
		midi->setCallback(midicallback);
	}
	catch (const RtError& err) {
		err.printMessage();
	}
	
	init_audio();
	
	Instrument::register_class("PadSynth", PadSynth::create);
	Instrument::register_class("Hyper Saw", HyperSaw::create);
	Instrument::register_class("CustomSynth", CustomSynth::create);
	Instrument::register_class("KickDrum", KickDrum::create);
	Instrument::register_class("PWMSynth", PWMSynth::create);
	
	default_font=Font::load_from_file("fonts/sans.ft");
	thin_font=Font::load_from_file("fonts/sansthin.ft");
	monospaced_font=Font::load_monospaced("fonts/sanserif.pgm", 8, 16);
	
	int shv=glsl_load_shader(GL_VERTEX_SHADER, "shader/text.vert");
	int shf=glsl_load_shader(GL_FRAGMENT_SHADER, "shader/text.frag");
	text_shader=glsl_link_program(shv, shf);
	
	int shg;
	shv=glsl_load_shader(GL_VERTEX_SHADER, "shader/glyph.vert");
	shg=glsl_load_shader(GL_GEOMETRY_SHADER_EXT, "shader/glyph.geom");
	shf=glsl_load_shader(GL_FRAGMENT_SHADER, "shader/glyph.frag");
	glyph_shader=glsl_link_program(shv, shg, shf, 8, GL_POINTS, GL_TRIANGLE_STRIP);
	
	Module* module;

	mixer=new Mixer();
	desktop->set_mixer(mixer);
	
	module=new Module;
	module->instruments[0]=new KickDrum(*mixer);
	module->instruments[0]->set_color(0xffaa55);
	module->instruments[1]=new PadSynth(*mixer, 0.25);
	module->instruments[1]->set_color(0xaa55ff);
	module->instruments[2]=new PadSynth(*mixer, 0.5);
	module->instruments[2]->set_color(0x55ffaa);
	module->instruments[3]=new PadSynth(*mixer, 1);
	module->instruments[4]=new PadSynth(*mixer, 0.25);
	module->instruments[5]=new PadSynth(*mixer, 0.5);
	module->instruments[6]=new PadSynth(*mixer, 1);
	module->instruments[7]=new AdditiveSynth(*mixer);
	module->instruments[8]=new FMSynth(*mixer);
	module->instruments[9]=new PWMSynth(*mixer);
	module->instruments[10]=new HyperSaw(*mixer);
	module->instruments[11]=new PitchlessSynth(*mixer);
	
	CustomSynth* customsynth=new CustomSynth(*mixer);
	customsynth->set_source("decay=exp(time*dc(-0.0001))\nx=sin(time*dc(440Hz)) + sin(time*dc(omega))\nleft=x*decay\nright=left\n");
	customsynth->compile();
	module->instruments[12]=customsynth;
	
	desktop->set_module(module);
	
	SDL_PauseAudio(0);

	desktop->main_loop();
	delete desktop;
	SDL_Quit();
	
	return 0;
}

