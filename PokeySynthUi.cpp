#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Table.H>
#include <FL/x.H>

#include <X11/Xlib.h>

#include "PokeySynth.h"

class PokeySynthUi {
public:
    PokeySynthUi(LV2UI_Write_Function write_function,
                 LV2UI_Controller controller,
                 void *parentWindow);
    void portEvent(uint32_t port_index,
                   uint32_t buffer_size,
                   uint32_t format,
                   const void *buffer);

    Fl_Window *window;
    void *parentWindow;
    Fl_Radio_Button *listenRadioButtons[4];

private:
    LV2UI_Write_Function write_function;
    LV2UI_Controller controller;

    void HandleListenCB(Fl_Widget *, void *data);
    static void HandleListenCB_redirect(Fl_Widget *, void *data);
};

// ****************************************************************************

void PokeySynthUi::HandleListenCB(Fl_Widget *, void *data) {
    PokeySynthUi *ui = (PokeySynthUi *) data;
    float which = 0;
    for (int x=0; x<4; x++) {
        if(ui->listenRadioButtons[x]->value()) {
            which = x;
            break;
        }
    }
    write_function(controller, POKEYSYNTH_CONTROL_CHANNELS, sizeof(float),
            0, (const void*) &which);
}

void PokeySynthUi::HandleListenCB_redirect(Fl_Widget *w, void *data) {
    ((PokeySynthUi *) data)->HandleListenCB(w,data);
}

class Label : public Fl_Box {
    public:
    Label(int x, int y, int w, int h, const char *label = nullptr)
        : Fl_Box(x,y,w,h,label) {
        box(FL_FLAT_BOX);
        labelsize(14);
    }
};

class Separator : public Fl_Box {
    public:
    Separator(int y, int w, const char *label=nullptr) : Fl_Box(0,y,w,1,label) {
        color(FL_BLACK);
        box(FL_FLAT_BOX);
    }
};

PokeySynthUi::PokeySynthUi(LV2UI_Write_Function write_function,
                           LV2UI_Controller controller,
                           void *parentWindow) :
    parentWindow(parentWindow),
    write_function(write_function),
    controller(controller) {

    Fl::visual(FL_DOUBLE|FL_INDEX);

    window = new Fl_Double_Window(640,480);

    Fl_Box *title = new Fl_Box(0,0,window->w(),48, "PokeySynth");
    title->labelfont(FL_BOLD+FL_ITALIC);
    title->labelsize(40);

    Fl_Box *copyright = new Fl_Box(0, title->x()+title->h(),
                                   window->w(), 20,
                                   "Version 0.9.0 / Copyright © 2025 "
                                   "by Ivo van Poorten");
    copyright->labelfont(FL_ITALIC);
    copyright->labelsize(14);

    new Separator(copyright->y() + copyright->h(), window->w());

    Fl_Group *group1 = new Fl_Group(64,copyright->y()+copyright->h()+8,512,24);
    group1->begin(); {
        int y = group1->y();
        const char *t[4] = {
            "Channel 1-4", "Channel 5-8", "Channel 9-12", "Channel 13-16"
        };
        for (int x=0; x<4; x++) {
            listenRadioButtons[x] = new Fl_Radio_Button(64+x*128,y,128,24,t[x]);
            listenRadioButtons[x]->callback(HandleListenCB_redirect, this);
        }
    }
    group1->end();

    window->size_range(window->w(),window->h(),window->w(),window->h());
    window->end();
    window->show();

    Window w = fl_xid(window);

    printf("debug: window = %llx\n", (unsigned long long ) w);
    printf("debug: parentWindow = %llx\n", (unsigned long long) parentWindow);

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
    #error "WIN32 not implemented yet"
#else
    #error "Unsupported platform"
#endif
}

// ****************************************************************************

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

    void* parentWindow = nullptr;
    for (int i = 0; features[i]; ++i) {
        if (strcmp (features[i]->URI, LV2_UI__parent) == 0)
            parentWindow = features[i]->data;
    }

    if (!parentWindow) {
        fprintf(stderr, "Required feature LV2_UI__parent not provided\n");
        return nullptr;
    }

    PokeySynthUi *ui;
    try {
        ui = new PokeySynthUi(write_function, controller, parentWindow);
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
