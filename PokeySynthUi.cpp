#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <lv2/core/lv2.h>
#include <lv2/ui/ui.h>
#include <lv2/atom/forge.h>
#include <lv2/midi/midi.h>

#include "fltk.h"

#include <X11/Xlib.h>

#include "PokeySynth.h"
#include "PokeyInstrument.h"
#include "InstrumentEditor.h"
#include "UiHelpers.h"

struct pokey_instrument (*instr)[128] = nullptr;

class PokeySynthUi {
public:
    PokeySynthUi(LV2UI_Write_Function write_function,
                 LV2UI_Controller controller,
                 const LV2_Feature *const *features);
    void portEvent(uint32_t port_index,
                   uint32_t buffer_size,
                   uint32_t format,
                   const void *buffer);

    Fl_Window *window;
    void *parentWindow;
    InstrumentEditor *editor;

private:
    LV2UI_Write_Function write_function;
    LV2UI_Controller controller;
    LV2_Atom_Forge forge;
    LV2_URID_Map *map;

    struct {
        LV2_URID midi_MidiEvent;
        LV2_URID instrdata_pointer;
    } uris;

    Fl_Radio_Button *listenRadioButtons[4];
    static void HandleListenCB_redirect(Fl_Widget *w, void *data);
    void HandleListenCB(Fl_Widget *w, void *data);

    Fl_Radio_Button *modesRadioButtons[4][3];
    static void HandleModesCB_redirect(Fl_Widget *w, void *data);
    void HandleModesCB(Fl_Widget *w, void *data);

    Fl_Hor_Value_Slider *arpSpeedSliders[4];
    static void HandleArpSpeedCB_redirect(Fl_Widget *w, void *data);
    void HandleArpSpeedCB(Fl_Widget *w, void *data);

    Fl_Radio_Button *updateSpeedRadioButtons[4];
    static void HandleUpdateSpeedCB_redirect(Fl_Widget *w, void *data);
    void HandleUpdateSpeedCB(Fl_Widget *w, void *data);
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

// ****************************************************************************
//
// MAIN GUI Constructor / Instantiate
//
PokeySynthUi::PokeySynthUi(LV2UI_Write_Function write_function,
                           LV2UI_Controller controller,
                           const LV2_Feature *const *features) :
    write_function(write_function),
    controller(controller) {

    for (int i = 0; features[i]; ++i) {
        if (!strcmp (features[i]->URI, LV2_UI__parent)) {
            parentWindow = features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_URID__map)) {
            map = (LV2_URID_Map*)features[i]->data;
      }
    }

    uris.midi_MidiEvent = map->map(map->handle, LV2_MIDI__MidiEvent);
    uris.instrdata_pointer = map->map(map->handle, POKEYSYNTH_URI"#instrdata_pointer");

    // setup UI

    int cury = 32, curx = 0;

    Fl::visual(FL_DOUBLE|FL_INDEX);

#define WIDTH 800
#define HEIGHT 600

    window = new Fl_Double_Window(WIDTH, HEIGHT);

    Fl_Tabs *tabs = new Fl_Tabs(0, 0, WIDTH, HEIGHT);

    Fl_Group *tab2 = new Fl_Group(0, cury, WIDTH, HEIGHT, "Editor");
    tab2->hide();
    editor = new InstrumentEditor(window->w()-2);
    tab2->end();

    Fl_Group *tab3 = new Fl_Group(0, cury, WIDTH, HEIGHT, "Load");
    new Fl_Button(32,64,128,24,"Hallo");
    new Fl_Button(256,64,128,24,"Doei");
    tab3->hide();
    tab3->end();

    Fl_Group *tab1 = new Fl_Group(0, cury, WIDTH, HEIGHT, "Main");

    cury += 16;

    Fl_Box *title = new Fl_Box(0,cury,window->w()-2,48, "PokeySynth");
    title->labelfont(FL_BOLD+FL_ITALIC);
    title->labelsize(40);

    Fl_Box *copyright = new Fl_Box(0, title->y()+title->h(),
                                   window->w()-2, 20,
                                   "Version 0.9.0 / Copyright © 2025 "
                                   "by Ivo van Poorten");
    copyright->labelfont(FL_ITALIC);
    copyright->labelsize(14);

    cury = copyright->y() + copyright->h();

    new Separator(cury, window->w()-2);

    new Label(cury+8, window->w()-2, "MIDI Channels");

    cury += 28;

    curx = (WIDTH-512) / 2;

    Fl_Group *group1 = new Fl_Group(curx,cury,512,24);
    group1->begin(); {
        int y = group1->y();
        const char *t[4] = {
            "Channel 1-4", "Channel 5-8", "Channel 9-12", "Channel 13-16"
        };
        for (int x=0; x<4; x++) {
            listenRadioButtons[x] =
                            new Fl_Radio_Button(curx+x*128, y, 128, 24, t[x]);
            listenRadioButtons[x]->callback(HandleListenCB_redirect, this);
        }
    }
    group1->end();

    cury += group1->h() + 8;
    new Separator(cury, window->w()-2);
    new Label(cury+8, window->w()-2, "Pokey Channels");

    cury += 28;

    {
        char s[40];
        const char *t[3] = { "Monophonic", "Arpeggiate Up", "Arpeggiate Down" };
        for (int c=0; c<4; c++) {
            snprintf(s, 40, "Channel %d:", c+1);
            new Label(curx, cury+c*24, 128, 24, strdup(s));
            Fl_Group *group = new Fl_Group(192, cury+c*24, 384, 24);
            group->begin();
                for (int x=0; x<3; x++) {
                    modesRadioButtons[c][x] =
                        new Fl_Radio_Button(curx+128+x*128, cury+c*24,
                                            128, 24, t[x]);
                    modesRadioButtons[c][x]->callback(HandleModesCB_redirect,
                                                                        this);
                }
            group->end();
        }
    }

    cury += 96 + 8;
    new Label(cury, window->w()-2, "Arpeggiate Speed");
    cury += 20;

    for (int x=0; x<4; x++) {
        char s[40];
        snprintf(s, 40, "Channel %d", x+1);
        arpSpeedSliders[x] =
                        new ArpSlider(curx+x*128, cury, 128, 24, strdup(s));
        arpSpeedSliders[x]->callback(HandleArpSpeedCB_redirect, this);
    }

    cury += 48;
    new Label(cury, window->w()-2, "Update Speed");
    cury += 20;

    Fl_Group *group3 = new Fl_Group(curx,cury, 512, 24);
    group3->begin(); {
        const char *t[4] = { "50Hz", "100Hz", "150Hz", "200Hz" };
        for (int x=0; x<4; x++) {
            updateSpeedRadioButtons[x] =
                new Fl_Radio_Button(curx+x*128, cury, 128, 24, t[x]);
            updateSpeedRadioButtons[x]->callback(HandleUpdateSpeedCB_redirect, this);
        }
    }
    group3->end();
    cury += 24 + 8;

    tab1->end();

    tabs->end();

    window->size_range(window->w(),window->h(),window->w(),window->h());
    window->end();
    window->show();

    if (!parentWindow) return;

    Window w = fl_xid(window);

//    printf("debug: window = %llx\n", (unsigned long long ) w);
//    printf("debug: parentWindow = %llx\n", (unsigned long long) parentWindow);

#ifdef __linux__
    XSync(fl_display, False);
    Fl::check();
    Fl::flush();
    usleep(100000);

    XUnmapWindow(fl_display, w);

    XSync(fl_display, False);
    Fl::check();
    Fl::flush();
    usleep(100000);

    XReparentWindow(fl_display, w, (Window) parentWindow, 0,0);
    XMapWindow(fl_display, w);
    XSync(fl_display, False);
#elif _WIN32
    // windows code goes here, reparent HWND
    #error "WIN32 support not implemented yet"
#elif __APPLE__
    // macOS code goes here, reparent NSView
    #error "macOS support not implemented yet"
#else
    #error "Unsupported platform"
#endif
}

// ****************************************************************************
//
// HANDLE INCOMING PORT EVENTS
//
void PokeySynthUi::portEvent(uint32_t port_index,
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
            LV2_Atom_Object *obj = (LV2_Atom_Object *) buffer;
            const LV2_Atom_Object *body = nullptr;

            lv2_atom_object_get(obj, uris.instrdata_pointer, &body, 0);
            if (obj) {
                unsigned long long v = *(unsigned long long *)LV2_ATOM_BODY(body);
                instr = (struct pokey_instrument (*)[128]) v;
            }
        }
        break;
    }
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
        ui = new PokeySynthUi(write_function, controller, features);
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
    if (psui) psui->portEvent(port_index, buffer_size, format, buffer);
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
        default:    return 0;
    }
}
