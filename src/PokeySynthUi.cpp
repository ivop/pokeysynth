// ****************************************************************************
//
// This file is part of PokeySynth.
//
// Copyright © 2024, 2025, by Ivo van Poorten
//
// Licensed under the terms of the General Public License, version 2.
// See the LICENSE file in the root of the prohect directory for the full text.
//
// ****************************************************************************

#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lv2.h"
#include "uris.h"
#include "fltk.h"
#include "platform.h"

#include "PokeySynth.h"
#include "PokeyInstrument.h"
#include "InstrumentEditor.h"
#include "UiHelpers.h"

class PokeySynthUi {
public:
    PokeySynthUi(LV2UI_Write_Function write_function,
                 LV2UI_Controller controller,
                 const LV2_Feature *const *features,
                 const char *bundle_path);
    ~PokeySynthUi(void);
    void PortEvent(uint32_t port_index,
                   uint32_t buffer_size,
                   uint32_t format,
                   const void *buffer);

    Fl_Window *window;
    void *parentWindow;
    InstrumentEditor *editor;
    Fl_Group *editorTab;

private:
    LV2UI_Write_Function write_function;
    LV2UI_Controller controller;
    const char *bundle_path;
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Frame frame;
    uint8_t atom_buffer[1024*128];
    LV2_URID_Map *map;

    char *sapr_filename;
    InputField *saprFilename;
    Fl_Box *saprStatus;

    FlatRadioButton *listenRadioButtons[4];
    static void HandleListenCB_redirect(Fl_Widget *w, void *data);
    void HandleListenCB(Fl_Widget *w, void *data);

    FlatRadioButton *modesRadioButtons[4][3];
    static void HandleModesCB_redirect(Fl_Widget *w, void *data);
    void HandleModesCB(Fl_Widget *w, void *data);

    MouseWheelSlider *arpSpeedSliders[4];
    static void HandleArpSpeedCB_redirect(Fl_Widget *w, void *data);
    void HandleArpSpeedCB(Fl_Widget *w, void *data);

    MouseWheelSlider *compensationSlider;
    static void HandleCompensationSlider_redirect(Fl_Widget *w, void *data);
    void HandleCompensationSlider(Fl_Widget *w, void *data);

    FlatRadioButton *updateSpeedRadioButtons[4];
    static void HandleUpdateSpeedCB_redirect(Fl_Widget *w, void *data);
    void HandleUpdateSpeedCB(Fl_Widget *w, void *data);

    static void Panic_redirect(Fl_Widget *w, void *data);
    void Panic(Fl_Widget *w, void *data);

    void SendShort(LV2_URID urid);
    void SendSaprFilename(void);

    static void HandleBrowseSapr_redirect(Fl_Widget *w, void *data);
    void HandleBrowseSapr(Fl_Widget *w, void *data);
    static void HandleSaprStart_redirect(Fl_Widget *w, void *data);
    void HandleSaprStart(Fl_Widget *w, void *data);
    static void HandleSaprStop_redirect(Fl_Widget *w, void *data);
    void HandleSaprStop(Fl_Widget *w, void *data);

    static void HandleSaprFilename_redirect(Fl_Widget *w, void *data);
    void HandleSaprFilename(Fl_Widget *w, void *data);

    struct pokey_instrument instrdata[128];
};

// ****************************************************************************
//
// CALLBACKS - Send data to controller
//
void PokeySynthUi::HandleListenCB_redirect(Fl_Widget *w, void *data) {
    ((PokeySynthUi *) data)->HandleListenCB(w,data);
}

void PokeySynthUi::HandleListenCB(Fl_Widget *, void *data) {
    float which = 0;
    for (int x=0; x<4; x++) {
        if(listenRadioButtons[x]->value()) {
            which = x;
            break;
        }
    }
    write_function(controller, POKEYSYNTH_CONTROL_CHANNELS, sizeof(float),
            0, (const void*) &which);
}

void PokeySynthUi::HandleModesCB_redirect(Fl_Widget *w, void *data) {
    ((PokeySynthUi *) data)->HandleModesCB(w,data);
}

void PokeySynthUi::HandleModesCB(Fl_Widget *w, void *data) {
    float which = 0;
    for (int y=0; y<4; y++) {
        for (int x=0; x<3; x++) {
            if (modesRadioButtons[y][x]->value()) {
                which = x;
                break;
            }
        }
        write_function(controller, POKEYSYNTH_CONTROL_MONO_ARP1+y,
                sizeof(float), 0, (const void*) &which);
    }
}

void PokeySynthUi::HandleArpSpeedCB_redirect(Fl_Widget *w, void *data) {
    ((PokeySynthUi *) data)->HandleArpSpeedCB(w,data);
}

void PokeySynthUi::HandleArpSpeedCB(Fl_Widget *w, void *data) {
    for (int x=0; x<4; x++) {
        float v = arpSpeedSliders[x]->value();
        write_function(controller, POKEYSYNTH_CONTROL_ARP_SPEED1+x,
                sizeof(float), 0, (const void*) &v);
    }
}

void PokeySynthUi::HandleCompensationSlider_redirect(Fl_Widget *w, void *data){
    ((PokeySynthUi *) data)->HandleCompensationSlider(w,data);
}

void PokeySynthUi::HandleCompensationSlider(Fl_Widget *w, void *data) {
    float v = compensationSlider->value();
    write_function(controller, POKEYSYNTH_CONTROL_OVERDRIVE_COMP,
            sizeof(float), 0, (const void *) &v);
}

void PokeySynthUi::HandleUpdateSpeedCB_redirect(Fl_Widget *w, void *data) {
    ((PokeySynthUi *) data)->HandleUpdateSpeedCB(w,data);
}

void PokeySynthUi::HandleUpdateSpeedCB(Fl_Widget *w, void *data) {
    float which = 0;
    for (int x=0; x<4; x++) {
        if (updateSpeedRadioButtons[x]->value()) {
            which = x;
            break;
        }
    }
    write_function(controller, POKEYSYNTH_CONTROL_UPDATE_FREQ,
            sizeof(float), 0, (const void*) &which);
}

void PokeySynthUi::Panic_redirect(Fl_Widget *w, void *data) {
    ((PokeySynthUi *) data)->Panic(w,data);
}

void PokeySynthUi::Panic(Fl_Widget *w, void *data) {
    for (int c=0; c<16; c++) {
        struct {
            LV2_Atom atom;
            uint8_t msg[3];
        } midimsg;

        midimsg.atom.type = uris.midi_MidiEvent;
        midimsg.atom.size = 3;
        midimsg.msg[0] = 0xb0 + c;
        midimsg.msg[1] = 120;           // CC120 All Notes Off
        midimsg.msg[2] = 0;

        write_function(controller, POKEYSYNTH_MIDI_IN, sizeof(LV2_Atom) + 3,
                uris.atom_eventTransfer, &midimsg);

        midimsg.msg[0] = 0xb0 + c;
        midimsg.msg[1] = 0x01;          // CC1 ModWheel
        midimsg.msg[2] = 0;

        write_function(controller, POKEYSYNTH_MIDI_IN, sizeof(LV2_Atom) + 3,
                uris.atom_eventTransfer, &midimsg);

        midimsg.msg[0] = 0xb0 + c;
        midimsg.msg[1] = 0x07;          // CC7 Volume
        midimsg.msg[2] = 127;

        write_function(controller, POKEYSYNTH_MIDI_IN, sizeof(LV2_Atom) + 3,
                uris.atom_eventTransfer, &midimsg);
    }
}

// ****************************************************************************
//
// MAIN GUI Constructor / Instantiate
//

void EditorTabCB(Fl_Widget *w, void *data) {
    Fl_Tabs *t = (Fl_Tabs *) w;
    PokeySynthUi * psui = (PokeySynthUi *) data;
    if (t->value() == psui->editorTab) {
        psui->editor->DrawProgram();
    }
}

PokeySynthUi::PokeySynthUi(LV2UI_Write_Function write_function,
                           LV2UI_Controller controller,
                           const LV2_Feature *const *features,
                           const char *bundle_path) :
    write_function(write_function),
    controller(controller) {

    this->bundle_path = strdup(bundle_path);
    sapr_filename = nullptr;

    for (int i = 0; features[i]; ++i) {
        if (!strcmp (features[i]->URI, LV2_UI__parent)) {
            parentWindow = features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_URID__map)) {
            map = (LV2_URID_Map*)features[i]->data;
      }
    }

    init_uris(map);

    lv2_atom_forge_init(&forge, map);

    // setup UI

    int cury = 0, curx = 0, savex;

    Fl::visual(FL_DOUBLE|FL_INDEX);

#define WIDTH 1088
#define HEIGHT 888

    window = new Fl_Double_Window(WIDTH, HEIGHT);

    // ---------- TITLE ----------

    Fl_Box *title = new Fl_Box(16+WIDTH*3/5,56,WIDTH*2/5,56, "PokeySynth");
    title->labelfont(FL_BOLD+FL_ITALIC);
    title->labelsize(28);
    title->labelcolor(fl_rgb_color(0x20,0x40,0x80));

    Fl_Box *copyright = new Fl_Box(title->x(), title->y()+title->h()-8,
                                   title->w(), 20,
                                   "Version 0.9.0 / Copyright © 2025 "
                                   "by Ivo van Poorten");
    copyright->labelfont(FL_ITALIC);
    copyright->labelsize(10);

    // ---------- MIDI CHANNELS / UPDATE SPEED ----------

    new Label(curx, cury, WIDTH/2, "MIDI Channels");
    new Label((WIDTH/2) + curx, cury, WIDTH/2, "Update Speed");

    cury += 20;
    curx = (WIDTH/2 - 512) / 2;

    Fl_Group *group1 = new Fl_Group(curx,cury,512,24);
    group1->begin(); {
        int y = group1->y();
        const char *t[4] = {
            "Channel 1-4", "Channel 5-8", "Channel 9-12", "Channel 13-16"
        };
        for (int x=0; x<4; x++) {
            listenRadioButtons[x] =
                            new FlatRadioButton(curx+x*128, y, 128, 24, t[x]);
            listenRadioButtons[x]->callback(HandleListenCB_redirect, this);
        }
    }
    group1->end();

    savex = curx;
    curx += WIDTH/2;

    Fl_Group *group3 = new Fl_Group(curx,cury, 512, 24);
    group3->begin(); {
        const char *t[4] = { "50Hz", "100Hz", "150Hz", "200Hz" };
        for (int x=0; x<4; x++) {
            updateSpeedRadioButtons[x] =
                new FlatRadioButton(curx+x*128, cury, 128, 24, t[x]);
            updateSpeedRadioButtons[x]->callback(HandleUpdateSpeedCB_redirect,
                                                                        this);
        }
    }
    group3->end();

    cury += group1->h() + 8;
    new Separator(cury, window->w());
    cury += 4;
    curx = savex;

    // ---------- POKEY CHANNELS / ARPEGGIATE SPEED ----------

    new Label(0, cury, WIDTH/2, "Pokey Channels");
    new Label(WIDTH/2 + 16, cury, WIDTH/8, "Arp Speed");

    cury += 20;

    {
        char s[40];
        const char *t[3] = { "Monophonic", "Arpeggiate Up", "Arpeggiate Down" };
        for (int c=0; c<4; c++) {
            snprintf(s, 40, "Channel %d .............", c+1);
            Fl_Box *b = new Label(curx, cury+c*24, 128, 24, strdup(s));
            b->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            b->labelsize(b->labelsize()-1);
            Fl_Group *group = new Fl_Group(192, cury+c*24, 384, 24);
            group->begin();
                for (int x=0; x<3; x++) {
                    modesRadioButtons[c][x] =
                        new FlatRadioButton(curx+128+x*128, cury+c*24,
                                            128, 24, t[x]);
                    modesRadioButtons[c][x]->callback(HandleModesCB_redirect,
                                                                        this);
                }
            group->end();
        }
    }

    curx += WIDTH / 2;
    for (int y=0; y<4; y++) {
        arpSpeedSliders[y] =
                        new MouseWheelSlider(curx, cury+y*24, 128, 24);
        arpSpeedSliders[y]->callback(HandleArpSpeedCB_redirect, this);
    }

    Fl_Button *but = new Fl_Button(curx+3*128, cury+3*24-8, 128, 24, "Panic!");
    but->callback(Panic_redirect, this);
    but->clear_visible_focus();

    compensationSlider = new MouseWheelSlider(curx+224, cury+3*24-8,
                                        128, 16, "Overdrive Compensation");
    compensationSlider->labelsize(12);
    compensationSlider->minimum(0);
    compensationSlider->maximum(15);
    compensationSlider->step(1);
    compensationSlider->callback(HandleCompensationSlider_redirect, this);

    cury += 96 + 8;

    // ---------- INSTRUMENT EDITOR ----------

    new Separator(cury, window->w());
    cury += 4;

    editor = new InstrumentEditor(window->w(),
                                  cury,
                                  write_function,
                                  controller,
                                  map,
                                  this->bundle_path,
                                  instrdata);

    // ---------- SAP-R RECORDING ----------

    cury = HEIGHT - 32;

    new Separator(cury, window->w());
    cury += 8;

    curx = 16;

    auto pck = new Fl_Pack(curx, cury, WIDTH-32, 20);
    pck->type(FL_HORIZONTAL);
    pck->spacing(1);

    auto sapr = new Fl_Box(0, 0, 64, 24, "SAP-R");
    sapr->labelfont(FL_BOLD);
    sapr->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    auto fn = new Fl_Box(0, 0, 80, 24, "Filename:");
    fn->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    saprFilename = new InputField(0,0, 512-16, 24);
    saprFilename->callback(HandleSaprFilename_redirect, this);

    auto browse = new Fl_Button(0,0, 24, 24, "...");
    browse->callback(HandleBrowseSapr_redirect, this);

    auto status = new Fl_Box(0,0,64,24, "Status:");
    status->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

//    saprStatus = new Fl_Box(0,0,80,24, "Recording");
//    saprStatus->labelcolor(FL_RED);
    saprStatus = new Fl_Box(0,0,80,24, "Stopped");
    saprStatus->labelcolor(fl_rgb_color(0x20,0x80,0x40));
    saprStatus->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    auto trg = new Fl_Box(0, 0, 112, 24, "Trigger: CC14");
    trg->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    auto start = new Fl_Button(0,0, 64, 24, "Start");
    start->callback(HandleSaprStart_redirect, this);
    auto stop = new Fl_Button(0,0, 64, 24, "Stop");
    stop->callback(HandleSaprStop_redirect, this);

    pck->end();

    // END

    window->size_range(window->w(),window->h(),window->w(),window->h());
    window->end();
    window->show();

    SendShort(uris.request_filenames);

    if (!parentWindow) return;

    ReparentWindow(fl_xid(window), (Window) parentWindow);
}

PokeySynthUi::~PokeySynthUi(void) {
    puts("ui: destructor");
    if (editor->is_dirty()) {
       int answer = fl_choice("There are unsaved edits.\n\n"
              "Note that closing without saving will instruct\n"
              "the DSP to reload the uneditted bank\n"
              "from disk!\n", "Close without saving", "Save Bank", nullptr);
        if (answer == 1) {
            if (!editor->SaveBank()) {
                // if saving fails we might be in big trouble, but try
                // to reload anyway. if it's only a permission problem,
                // reload should succeed.
                editor->SendReloadFromFileToDSP();
            }
        } else {
            editor->SendReloadFromFileToDSP();
        }
    }
    Fl::check();
    Fl::flush();
}

// ****************************************************************************
//
// HANDLE INCOMING PORT EVENTS
//
void PokeySynthUi::PortEvent(uint32_t port_index,
                             uint32_t buffer_size,
                             uint32_t format,
                             const void *buffer) {
    float v = *((float*)buffer);
    int vi = v;
    switch (port_index) {
    case POKEYSYNTH_CONTROL_CHANNELS:
        listenRadioButtons[vi]->setonly();
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP1:
        modesRadioButtons[0][vi]->setonly();
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP2:
        modesRadioButtons[1][vi]->setonly();
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP3:
        modesRadioButtons[2][vi]->setonly();
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP4:
        modesRadioButtons[3][vi]->setonly();
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED1:
        arpSpeedSliders[0]->value(vi);
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED2:
        arpSpeedSliders[1]->value(vi);
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED3:
        arpSpeedSliders[2]->value(vi);
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED4:
        arpSpeedSliders[3]->value(vi);
        break;
    case POKEYSYNTH_CONTROL_UPDATE_FREQ:
        updateSpeedRadioButtons[vi]->setonly();
        break;
    case POKEYSYNTH_NOTIFY_GUI: {
        const LV2_Atom_Object* obj = (const LV2_Atom_Object*) buffer;
        if (obj->body.otype == uris.instrument_data) {
            puts("gui: received instrument data");
            const LV2_Atom *pgm = nullptr;
            const LV2_Atom *pgmdata = nullptr;

            lv2_atom_object_get(obj, uris.program_number, &pgm,
                                     uris.program_data, &pgmdata,
                                     0);
            if (pgm && pgmdata) {
                uint32_t program_number = ((const LV2_Atom_Int *)pgm)->body;
                const LV2_Atom_Vector *vec = (const LV2_Atom_Vector *)pgmdata;
                if (vec->body.child_type == uris.atom_Int) {
                    printf("gui: received program number %d\n", program_number);
                    uint8_t *data = (uint8_t *)(&vec->body + 1);
                    memcpy(&instrdata[program_number], data, sizeof(struct pokey_instrument));
                }
                if (program_number == editor->program) {
                    editor->DrawProgram();
                }
            }
        } else if (obj->body.otype == uris.filename_object) {
            puts("gui: received filename object");
            const LV2_Atom_String *bpath = nullptr;
            const LV2_Atom_String *spath = nullptr;

            lv2_atom_object_get(obj, uris.bank_filename, &bpath,
                                     uris.sapr_filename, &spath,
                                     0);
            if (bpath) {
                const char *f = (const char *)LV2_ATOM_BODY(bpath);
                editor->LoadBank(f);
            } 
            if (spath) {
                const char *f = (const char *)LV2_ATOM_BODY(spath);
                if (sapr_filename) {
                    free(sapr_filename);
                }
                sapr_filename = strdup(f);
                saprFilename->value(sapr_filename);
            }
        } else if (obj->body.otype == uris.start_sapr) {
            saprStatus->labelcolor(FL_RED);
            saprStatus->label("Recording");
        } else if (obj->body.otype == uris.stop_sapr) {
            saprStatus->labelcolor(FL_GREEN);
            saprStatus->label("Stopped");
        }
        break;
        }
    case POKEYSYNTH_CONTROL_OVERDRIVE_COMP:
        compensationSlider->value(v);
        break;
    default:
        break;
    }
}

// ****************************************************************************
// SEND MESSAGES TO DSP
//
void PokeySynthUi::SendShort(LV2_URID urid) {
    lv2_atom_forge_set_buffer(&forge, atom_buffer, sizeof(atom_buffer));

    LV2_Atom *msg = (LV2_Atom *) lv2_atom_forge_object(&forge,
                                                   &frame,
                                                   0,
                                                   urid);
    lv2_atom_forge_pop(&forge, &frame);
    write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void PokeySynthUi::SendSaprFilename(void) {
    lv2_atom_forge_set_buffer(&forge, atom_buffer, sizeof(atom_buffer));

    LV2_Atom *msg = (LV2_Atom *) lv2_atom_forge_object(&forge,
                                                       &frame,
                                                       0,
                                                       uris.filename_object);
    lv2_atom_forge_key(&forge, uris.sapr_filename);
    lv2_atom_forge_path(&forge, sapr_filename, strlen(sapr_filename));

    lv2_atom_forge_pop(&forge, &frame);
    write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

// ****************************************************************************
// BROWSE SAPR OUTPUT FILENAME
//
void PokeySynthUi::HandleBrowseSapr_redirect(Fl_Widget *w, void *data) {
    ((PokeySynthUi *) data)->HandleBrowseSapr(w,data);
}

void PokeySynthUi::HandleBrowseSapr(Fl_Widget *w, void *data) {
    auto path = saprFilename->value();
    auto newpath = fl_file_chooser("Save SAP-R to...", "*.sapr", path);
    if (newpath) {
        saprFilename->value(newpath);
        if (sapr_filename) {
            free(sapr_filename);
        }
        sapr_filename = strdup(newpath);
        SendSaprFilename();
    }
}

// ****************************************************************************
// SAPR FILENAME INPUT CALLBACK
//
void PokeySynthUi::HandleSaprFilename_redirect(Fl_Widget *w, void *data) {
    ((PokeySynthUi *) data)->HandleSaprFilename(w,data);
}

void PokeySynthUi::HandleSaprFilename(Fl_Widget *w, void *data) {
    if (sapr_filename) {
        free(sapr_filename);
    }
    sapr_filename = strdup(saprFilename->value());
    SendSaprFilename();
}

// ****************************************************************************
// SAP-R START/STOP BUTTONS
//
void PokeySynthUi::HandleSaprStart_redirect(Fl_Widget *w, void *data) {
    ((PokeySynthUi *) data)->HandleSaprStart(w,data);
}

void PokeySynthUi::HandleSaprStart(Fl_Widget *w, void *data) {
    SendShort(uris.start_sapr);
}

void PokeySynthUi::HandleSaprStop_redirect(Fl_Widget *w, void *data) {
    ((PokeySynthUi *) data)->HandleSaprStop(w,data);
}

void PokeySynthUi::HandleSaprStop(Fl_Widget *w, void *data) {
    SendShort(uris.stop_sapr);
}

// ****************************************************************************
//
// C API
//
static LV2UI_Handle instantiate(const struct LV2UI_Descriptor *descriptor,
                                const char *plugin_uri,
                                const char *bundle_path,
                                LV2UI_Write_Function write_function,
                                LV2UI_Controller controller,
                                LV2UI_Widget *widget,
                                const LV2_Feature *const *features) {

    if (strcmp (plugin_uri, "https://github.com/ivop/pokeysynth") != 0) {
        return nullptr;
    }

    PokeySynthUi *ui;
    try {
      ui = new PokeySynthUi(write_function, controller, features, bundle_path);
    }
    catch (std::exception& exc) {
        fprintf(stderr, "UI instantiation failed.\n");
        return nullptr;
    }

    *widget = reinterpret_cast<LV2UI_Widget>(fl_xid(ui->window));

    return (LV2UI_Handle) ui;
}

// ----------------------------------------------------------------------------

static void cleanup (LV2UI_Handle ui) {
    PokeySynthUi *psui = static_cast<PokeySynthUi *>(ui);
    if (psui) delete psui;
}

// ----------------------------------------------------------------------------

static void port_event(LV2UI_Handle ui,
                       uint32_t port_index,
                       uint32_t buffer_size,
                       uint32_t format,
                       const void *buffer) {
    PokeySynthUi* psui = static_cast<PokeySynthUi *>(ui);
    if (psui) psui->PortEvent(port_index, buffer_size, format, buffer);
}

// ----------------------------------------------------------------------------

static int ui_idle(LV2UI_Handle ui) {
//    PokeySynthUi *psui = static_cast<PokeySynthUi *>(ui);
    Fl::check();
    Fl::flush();
    return 0;
}

// ----------------------------------------------------------------------------

static const void * extension_data (const char *uri) {
    static const LV2UI_Idle_Interface idle = { ui_idle };
    if (strcmp (uri, LV2_UI__idleInterface) == 0) return &idle;
    return nullptr;
}

// ----------------------------------------------------------------------------

static const LV2UI_Descriptor ui_descriptor = {
    "https://github.com/ivop/pokeysynth#ui",
    instantiate,
    cleanup,
    port_event,
    extension_data
};

// ----------------------------------------------------------------------------

LV2_SYMBOL_EXPORT const LV2UI_Descriptor * 	lv2ui_descriptor (uint32_t index) {
    switch (index) {
        case 0:     return &ui_descriptor;
        default:    return nullptr;
    }
}
