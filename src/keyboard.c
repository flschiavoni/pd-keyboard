// Written By Flávio Schiavoni and revised by Porres

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "m_pd.h"
#include "g_canvas.h"

#define MOUSE_PRESS 127
#define MOUSE_RELEASE -1
#define KEY_DOWN 127
#define KEY_UP -1

static t_class *keyboard_class;

static t_int keyboard_mapping[24] = {122, 115, 120, 100, 99, 118, 103, 98, 104, 110, 106,
         109, 90, 83, 88, 68, 67, 86, 71, 66, 72, 78, 74, 77};

typedef struct _keyboard{
    t_object x_obj;
    t_float velocity_input; // to store the second inlet values
    t_int width;
    t_int height;
    t_int octaves;
    t_int first_c;
    t_int low_c;
    t_float space;
    t_int keyb_play;
    t_outlet* x_out;
    t_glist *glist;
    t_canvas *canvas;
    t_int *notes; // To store which notes should be played
} t_keyboard;

// Set Properties
static void keyboard_setvalues(t_keyboard *x, t_floatarg space, t_floatarg height,
        t_floatarg octaves, t_floatarg low_c, t_float keyb_play){
    x->keyb_play = keyb_play;
    x->height = (height > 10) ? height : 10;
    x->octaves = (octaves > 1) ? octaves : 1;
    x->low_c = (low_c >= 0) ? low_c : 0;
    x->low_c = (low_c <= 8) ? low_c : 8;
    x->first_c = ((int)(x->low_c * 12)) + 12;
//    t_float space = width / (7 * x->octaves);
    x->space = (space > 7) ? space : 7;
    x->width = ((int)(x->space)) * 7 * (int)x->octaves;
    x->notes = getbytes(sizeof(t_int) * 12 * x->octaves);
    int i;
    for(i = 0 ; i < 12 * x->octaves ; i++)
        x->notes[i] = 0;
}

/* ------------------------- Methods ------------------------------*/

static void keyboard_play(t_keyboard* x){
    t_canvas * canvas = x->canvas;
    int i;
    for(i = 0 ; i < x->octaves * 12; i++){
        short key = i % 12;
        if(x->notes[i] > 0){ // play Keyb or mouse
            if( key != 1 && key != 3 && key !=6 && key != 8 && key != 10)
                sys_vgui(".x%lx.c itemconfigure %xrrk%d -fill #9999FF\n", canvas, x, i);
            else
                sys_vgui(".x%lx.c itemconfigure %xrrk%d -fill #6666FF\n", canvas, x, i);
            t_atom a[2];
            SETFLOAT(a, ((int)x->first_c) + i);
            SETFLOAT(a+1, x->notes[i]);
            outlet_list(x->x_out, &s_list, 2, a);
        }
        if(x->notes[i] < 0){ // stop play Keyb or mouse
            if( key != 1 && key != 3 && key !=6 && key != 8 && key != 10){
                if(x->first_c + i == 60) // Middle C
                    sys_vgui(".x%lx.c itemconfigure %xrrk%d -fill #F0FFFF\n", canvas, x, i);
                else
                    sys_vgui(".x%lx.c itemconfigure %xrrk%d -fill #FFFFFF\n", canvas, x, i);
                }
            else
                sys_vgui(".x%lx.c itemconfigure %xrrk%d -fill #000000\n", canvas, x, i);
            t_atom a[2];
            SETFLOAT(a, ((int)x->first_c) + i);
            SETFLOAT(a+1, x->notes[i] = 0);
            outlet_list(x->x_out, &s_list, 2, a);
        }
    }
}

void keyboard_float(t_keyboard *x, t_floatarg note){
    //Check if this note is part of the Keyboard
    if(note < x->first_c || note > x->first_c + (x->octaves * 12)){
        t_atom a[2];
        SETFLOAT(a, note);
        SETFLOAT(a+1, x->velocity_input);
        outlet_list(x->x_out, &s_list, 2, a);
    }
    else{
        x->notes[(int)(note - x->first_c)] = (x->velocity_input > 0) ? x->velocity_input : KEY_UP;
        keyboard_play(x);
    }
}

/* -------------------- MOUSE Events ----------------------------------*/

// Mouse release
static void keyboard_mouserelease(t_keyboard* x, t_float xpix, t_float ypix){
    if (x->glist->gl_edit) // If edit mode, give up!
        return;
    int i, play;
    play = 0;
    for(i = 0 ; i < x->octaves * 12; i++){
        if(x->notes[i] == MOUSE_PRESS){
            x->notes[i] = MOUSE_RELEASE;
            play = 1;
            }
    }
    if(play == 1)
        keyboard_play(x);
}

//Map mouse event position
static int keyboard_mapclick(t_keyboard* x, t_float xpix, t_float ypix, t_int event){
    short i, wcounter, bcounter;
    wcounter = bcounter = 0;
    for(i = 0 ; i < x->octaves * 12 ; i++){
        short key = i % 12;
        if(key == 4 || key == 11){
            bcounter++;
        }
        if( key == 1 || key == 3 || key ==6 || key == 8 || key == 10){ // Play the blacks
            if(xpix > x->x_obj.te_xpix + ((bcounter + 1) * (int)x->space) - ((int)(0.3f * x->space))
                 && xpix < x->x_obj.te_xpix + ((bcounter + 1) * (int)x->space) + ((int)(0.3f * x->space))
                 && ypix > x->x_obj.te_ypix
                 && ypix < x->x_obj.te_ypix + 2 * x->height / 3
            ){
                x->notes[i] = event;
                return i; // Avoid to play the white below
            }
        bcounter++;
        }else{
            if(xpix > x->x_obj.te_xpix + wcounter * (int)x->space
                 && xpix < x->x_obj.te_xpix + (wcounter + 1) * (int)x->space
                 && ypix > x->x_obj.te_ypix
                 && ypix < x->x_obj.te_ypix + x->height){
                x->notes[i] = event;
                return i;
            }
        wcounter++;
        }
    }
    return -1;
}

// Mouse press
static void keyboard_mousepress(t_keyboard* x, t_float xpix, t_float ypix){
    if (x->glist->gl_edit) // If edit mode, give up!
        return;
    if((int)xpix < x->x_obj.te_xpix + 40
       || (int)xpix > x->x_obj.te_xpix + x->width + 40
       || (int)ypix < x->x_obj.te_ypix
       || (int)ypix > x->x_obj.te_ypix + x->height)
            return;
    keyboard_mapclick(x, xpix, ypix, MOUSE_PRESS);
    keyboard_play(x);
}

// Mouse Drag event
static void keyboard_mousemotion(t_keyboard* x, t_float xpix, t_float ypix){
    if (x->glist->gl_edit) // If edit mode, give up!
        return;
    if((int)xpix < x->x_obj.te_xpix + 40
       || (int)xpix > x->x_obj.te_xpix + x->width + 40
       || (int)ypix < x->x_obj.te_ypix
       || (int)ypix > x->x_obj.te_ypix + x->height)
            return;
    int i,actual = 0, new_drag = 0;
    for(i = 0 ; i < x->octaves * 12; i++){ // find actual key playing;
        if(x->notes[i] == MOUSE_PRESS){
            actual = i;
            break;
        }
    }
    new_drag = keyboard_mapclick(x, (int)xpix, (int)ypix, MOUSE_PRESS);
    if(new_drag != -1 && actual != new_drag){
        x->notes[actual] = MOUSE_RELEASE;
        keyboard_play(x);
    }
}

/* ------------------------ KEYBOARD events ---------------------------*/

// Key up event: Stop playing
static void keyboard_keyup(t_keyboard* x, t_float key){
    if(x->keyb_play == 0)
        return;
    int i, play;
    play = 0;
    for(i = 0 ; i < 24; i++)
        if(keyboard_mapping[i] == (t_int)(key)){
        x->notes[i] = KEY_UP;
        play = 1;
        }
    if(play == 1)
        keyboard_play(x);
}

//keydown event: PLAY
static void keyboard_keydown(t_keyboard* x, t_float key){
    if(x->keyb_play == 0)
        return;
    int i, play;
    play = 0;
    for(i = 0 ; i < 24; i++)
        if(keyboard_mapping[i] == (int)(key)){
        x->notes[i] = KEY_DOWN;
        play = 1;
        }
    if(play == 1)
        keyboard_play(x);
}

/* ------------------------ GUI Definitions ---------------------------*/

// THE BOUNDING RECTANGLE
static void keyboard_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2){
    t_keyboard *x = (t_keyboard *)z;
     *xp1 = x->x_obj.te_xpix;
     *yp1 = x->x_obj.te_ypix;
     *xp2 = x->x_obj.te_xpix + x->width;
     *yp2 = x->x_obj.te_ypix + x->height;
}

// Erase the keyboard
static void keyboard_erase(t_keyboard *x){
    t_canvas * canvas = x->canvas;
    sys_vgui(".x%lx.c delete %xrr\n",canvas, x);
    int i;
    for(i = 0 ; i < x->octaves * 12 ; i++){
        sys_vgui(".x%lx.c delete %xrrk%d\n", canvas, x, i);
    }
}

//Draw the keyboard
static void keyboard_draw(t_keyboard *x){
    t_canvas * canvas = x->canvas;
// Main rectangle
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %xrr -fill #FFFFFF\n",
        canvas,
        x->x_obj.te_xpix,
        x->x_obj.te_ypix,
        x->x_obj.te_xpix + x->width,
        x->x_obj.te_ypix + x->height,
        x
        );
    int i, wcounter, bcounter;
    wcounter = bcounter = 0;
// First draw the white keys to allow the blacks overlay it
    for(i = 0 ; i < x->octaves * 12 ; i++){
        short key = i % 12;
        if(key != 1 && key != 3 && key !=6 && key != 8 && key != 10){
            if(x->first_c + i == 60){ // Middle C
                sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %xrrk%d -fill #F0FFFF\n",
                         canvas,
                         x->x_obj.te_xpix + wcounter * (int)x->space,
                         x->x_obj.te_ypix,
                         x->x_obj.te_xpix + (wcounter + 1) * (int)x->space,
                         x->x_obj.te_ypix + x->height,
                         x,
                         i
                         );
                wcounter++;
            }
            else{
                sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %xrrk%d -fill #FFFFFF\n",
                canvas,
                x->x_obj.te_xpix + wcounter * (int)x->space,
                x->x_obj.te_ypix,
                x->x_obj.te_xpix + (wcounter + 1) * (int)x->space,
                x->x_obj.te_ypix + x->height,
                x,
                i
                );
                wcounter++;
            }
        }
    }
    for(i = 0 ; i < x->octaves * 12 ; i++){
        short key = i % 12;
        if(key == 4 || key == 11){
            bcounter++;
            continue;
        }
        if( key == 1 || key == 3 || key ==6 || key == 8 || key == 10){
// Draw the black keys
            sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %xrrk%d -fill #000000\n",
            canvas,
            x->x_obj.te_xpix + ((bcounter + 1) * (int)x->space) - ((int)(0.3f * x->space)) ,
            x->x_obj.te_ypix,
            x->x_obj.te_xpix + ((bcounter + 1) * (int)x->space) + ((int)(0.3f * x->space)) ,
            x->x_obj.te_ypix + 2 * x->height / 3,
            x,
            i
            );
            bcounter++;
        }
    }
    canvas_fixlinesfor(x->glist, (t_text *)x);
}

// Apply the changes of property windows
static void keyboard_apply(t_keyboard *x, t_floatarg space, t_floatarg height,
                           t_floatarg octaves, t_floatarg low_c, t_float keyb_play){
    keyboard_erase(x);
    keyboard_setvalues(x, space, height, octaves, low_c, keyb_play);
    keyboard_draw(x);
}

// MAKE VISIBLE OR INVISIBLE
static void keyboard_vis(t_gobj *z, t_glist *glist, int vis){
    t_keyboard *x = (t_keyboard *)z;

    t_canvas * canvas = glist_getcanvas(glist);
    x->glist = glist;
    x->canvas = canvas;

    if(vis == 0){ // INVISIBLE
        keyboard_erase(x);
        return;
    }
    if(vis == 1){
        keyboard_draw(x);
    }
}

// DISPLACE IT
void keyboard_displace(t_gobj *z, t_glist *glist,int dx, int dy){
    t_canvas * canvas = glist_getcanvas(glist);
    t_keyboard *x = (t_keyboard *)z;
    x->x_obj.te_xpix += dx; // x movement
    x->x_obj.te_ypix += dy; // y movement

    sys_vgui(".x%lx.c coords %xSEL %d %d %d %d \n", //MOVE THE BLUE ONE
        canvas, x,
        x->x_obj.te_xpix,
        x->x_obj.te_ypix,
        x->x_obj.te_xpix + x->width,
        x->x_obj.te_ypix + x->height
        );
    // MOVE the main rectangle
    sys_vgui(".x%x.c coords %xrr %d %d %d %d\n",
        canvas, x,
        x->x_obj.te_xpix,
        x->x_obj.te_ypix,
        x->x_obj.te_xpix + x->width,
        x->x_obj.te_ypix + x->height
        );
    // MOVE THE KEYS
    int i, wcounter, bcounter;
    wcounter = bcounter = 0;
    for(i = 0 ; i < x->octaves * 12 ; i++){
        short key = i % 12;
        if(key == 4 || key == 11)//Increment black keys counter
            bcounter++;
        if( key == 0 || key == 2 || key ==4 || key == 5 || key == 7 || key == 9 || key == 11){
        // Draw the white keys
            sys_vgui(".x%lx.c coords %xrrk%d %d %d %d %d \n",
                canvas, x, i,
                x->x_obj.te_xpix + wcounter * (int)x->space,
                x->x_obj.te_ypix,
                x->x_obj.te_xpix + (wcounter + 1) * (int)x->space,
                x->x_obj.te_ypix + x->height
                );
            wcounter++;
        }else{
            sys_vgui(".x%lx.c coords %xrrk%d %d %d %d %d \n",
                canvas, x, i,
                x->x_obj.te_xpix + ((bcounter + 1) * (int)x->space) - ((int)(0.3f * x->space)) ,
                x->x_obj.te_ypix,
                x->x_obj.te_xpix + ((bcounter + 1) * (int)x->space) + ((int)(0.3f * x->space)) ,
                x->x_obj.te_ypix + 2 * x->height / 3
                );
            bcounter++;
        }
    }
    canvas_fixlinesfor(glist, (t_text *)x);
}

// WHAT TO DO IF SELECTED?
static void keyboard_select(t_gobj *z, t_glist *glist, int state){
     t_keyboard *x = (t_keyboard *)z;
     t_canvas * canvas = glist_getcanvas(glist);
     if (state) {
        sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xSEL -outline blue\n",
        canvas,
        x->x_obj.te_xpix,
        x->x_obj.te_ypix,
        x->x_obj.te_xpix + x->width,
        x->x_obj.te_ypix + x->height,
        x
        );
    }else {
     sys_vgui(".x%x.c delete %xSEL\n",canvas, x);
    }
}

// Delete the keyboard
static void keyboard_delete(t_gobj *z, t_glist *glist){
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);
} 

/* ------------------------ GUI Behaviour -----------------------------*/

void keyboard_properties(t_gobj *z, t_glist *owner){
    t_keyboard *x = (t_keyboard *)z;
    char cmdbuf[256];
    sprintf(cmdbuf, "keyboard_properties %%s %d %d %d %d %d\n", 
        (int)x->space,
        (int)x->height, 
        (int)x->octaves,
        (int)x->low_c,
        (int)x->keyb_play);
    gfxstub_new(&x->x_obj.ob_pd, x, cmdbuf);
}

static void keyboard_save(t_gobj *z, t_binbuf *b){
    t_keyboard *x = (t_keyboard *)z;
   binbuf_addv(b, "ssiisiiiii",gensym("#X"),gensym("obj"),
        (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
        gensym("keyboard"),
        (t_int)x->space,
        (t_int)x->height,
        (t_int)x->octaves,
        (t_int)x->low_c,
        (t_int)x->keyb_play);
   binbuf_addv(b, ";");
}

t_widgetbehavior keyboard_widgetbehavior ={
    keyboard_getrect,
    keyboard_displace,
    keyboard_select,
    NULL, //Activate will not be used
    keyboard_delete,
    keyboard_vis,
    NULL, //Will use PD default mouse click
};

/* ------------------------ Free / New / Setup ------------------------------*/

// Free
void keyboard_free(t_keyboard *x) {
    sys_vgui("event delete <<%x_mousedown>>\n", x);
    sys_vgui("event delete <<%x_mousemotion>>\n", x);
    sys_vgui("event delete <<%x_mouseup>>\n", x);
    sys_vgui("event delete <<%x_keydown>>\n", x);
    sys_vgui("event delete <<%x_keyup>>\n", x);
    pd_unbind(&x->x_obj.ob_pd, gensym("keyboard"));
    gfxstub_deleteforkey(x);
}

// New
void * keyboard_new(t_symbol *selector, int ac, t_atom* av){
    t_keyboard *x = (t_keyboard *) pd_new(keyboard_class);
    t_float init_space = 16;
    t_float init_height = 80;
    t_float init_8ves = 4;
    t_float init_low_c = 3;
    t_int init_computer_play = 0;
    if(ac) // 1st ARGUMENT IS WIDTH
        init_space = atom_getfloat(av++), ac--;
    if(ac) // 2nd ARGUMENT IS HEIGHT
        init_height = atom_getfloat(av++), ac--;
    if(ac) // 3rd ARGUMENT IS Octaves
        init_8ves = atom_getfloat(av++), ac--;
    if(ac) // 4th ARGUMENT IS Lowest C ("First C")
        init_low_c = atom_getfloat(av++), ac--;
    if(ac) // 5th ARGUMENT IS computer keyboard play enabled
        init_computer_play = atom_getfloat(av++) != 0, ac--;
    x->x_out = outlet_new(&x->x_obj, &s_list);
    floatinlet_new(&x->x_obj, &x->velocity_input);
// Set Parameters
    keyboard_setvalues(x, init_space, init_height, init_8ves, init_low_c, init_computer_play);
// GUI definitions
    pd_bind(&x->x_obj.ob_pd, gensym("keyboard"));
    
    sys_vgui("event add <<%x_mousedown>> <ButtonPress>\n", x);
    sys_vgui("proc %x_keyboard_mousepress {x y b} {\n if {$b == 1} {\npd [concat keyboard _mousepress $x $y\\;]\n}}\n", x);
    sys_vgui("bind all <<%x_mousedown>> {\n %x_keyboard_mousepress %%x %%y %%b\n}\n", x, x);
    
    sys_vgui("event add <<%x_mouseup>> <ButtonRelease>\n", x);
    sys_vgui("proc %x_keyboard_mouserelease {x y b} {\n if {$b == 1} {\npd [concat keyboard _mouserelease $x $y\\;]\n}}\n", x);
    sys_vgui("bind all <<%x_mouseup>> {\n %x_keyboard_mouserelease %%x %%y %%b\n}\n", x, x);
    
    sys_vgui("event add <<%x_mousemotion>> <B1-Motion>\n", x);
    sys_vgui("proc %x_keyboard_mousemotion {x y} { \n pd [concat keyboard _mousemotion $x $y\\;]\n}\n", x);
    sys_vgui("bind all <<%x_mousemotion>> {\n %x_keyboard_mousemotion %%x %%y\n}\n", x, x);
    
    sys_vgui("event add <<%x_keydown>> <KeyPress>\n", x);
    sys_vgui("proc %x_keyboard_keydown {K} {\n pd [concat keyboard _kdown $K\\;]\n}\n", x);
    sys_vgui("bind all <<%x_keydown>> {\n %x_keyboard_keydown %%N\n}\n", x, x);
    
    sys_vgui("event add <<%x_keyup>> <KeyRelease>\n", x);
    sys_vgui("proc %x_keyboard_keyup {K} {\n pd [concat keyboard _kup $K\\;]\n}\n", x);
    sys_vgui("bind all <<%x_keyup>> {\n %x_keyboard_keyup %%N\n}\n", x, x);
    return (void *) x;
}

// Setup
void keyboard_setup(void) {
    keyboard_class = class_new(gensym("keyboard"),
        (t_newmethod) keyboard_new,
        (t_method) keyboard_free,
        sizeof (t_keyboard),
        CLASS_DEFAULT,
        A_GIMME,
        0);
    class_addfloat(keyboard_class, keyboard_float);
// Methods to receive TCL/TK events
    class_addmethod(keyboard_class, (t_method)keyboard_mousepress,gensym("_mousepress"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(keyboard_class, (t_method)keyboard_mouserelease,gensym("_mouserelease"), A_FLOAT, A_FLOAT, 0); 
    class_addmethod(keyboard_class, (t_method)keyboard_mousemotion,gensym("_mousemotion"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(keyboard_class, (t_method)keyboard_keyup,gensym("_kup"), A_FLOAT, 0); 
    class_addmethod(keyboard_class, (t_method)keyboard_keydown,gensym("_kdown"), A_FLOAT, 0);

    class_setwidget(keyboard_class, &keyboard_widgetbehavior);
    class_setsavefn(keyboard_class, keyboard_save);
    class_setpropertiesfn(keyboard_class, keyboard_properties);
    class_addmethod(keyboard_class, (t_method)keyboard_apply, gensym("apply"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);

// Adjust from Vanilla e Extended
    sys_gui(" if { [catch {pd}] } {proc pd {args} {pdsend [join $args " "]}}\n");

// Properties dialog
    sys_gui("proc keyboard_properties {id width height octaves low_c keyb_play} {\n");
    sys_gui("set vid [string trimleft $id .]\n");
    sys_gui("set var_width [concat var_width_$vid]\n");
    sys_gui("set var_height [concat var_height_$vid]\n");
    sys_gui("set var_octaves [concat var_octaves_$vid]\n");
    sys_gui("set var_low_c [concat var_low_c_$vid]\n");
    sys_gui("set var_keyb_play [concat var_keyb_play_$vid]\n");

    sys_gui("global $var_width\n");
    sys_gui("global $var_height\n");
    sys_gui("global $var_octaves\n");
    sys_gui("global $var_low_c\n");
    sys_gui("global $var_keyb_play\n");

    sys_gui("set $var_width $width\n");
    sys_gui("set $var_height $height\n");
    sys_gui("set $var_octaves $octaves\n");
    sys_gui("set $var_low_c $low_c\n");
    sys_gui("set $var_keyb_play $keyb_play\n");

    sys_gui("toplevel $id\n");
    sys_gui("wm title $id {Keyboard}\n");
    sys_gui("wm protocol $id WM_DELETE_WINDOW [concat keyboard_cancel $id]\n");

    sys_gui("label $id.label -text {Keyboard}\n");
    sys_gui("pack $id.label -side top\n");

    sys_gui("frame $id.size\n");
    sys_gui("pack $id.size -side top\n");
    sys_gui("label $id.size.lwidth -text \"Key Width:\"\n");
    sys_gui("entry $id.size.width -textvariable $var_width -width 7\n");
    sys_gui("label $id.size.lheight -text \"Height:\"\n");
    sys_gui("entry $id.size.height -textvariable $var_height -width 7\n");
    sys_gui("pack $id.size.lwidth $id.size.width $id.size.lheight $id.size.height -side left\n");

    sys_gui("frame $id.notes\n");
    sys_gui("pack $id.notes -side top\n");
    sys_gui("label $id.notes.loctaves -text \"Octaves:\"\n");
    sys_gui("entry $id.notes.octaves -textvariable $var_octaves -width 7\n");
    sys_gui("label $id.notes.llow_c -text \"Low C:\"\n");
    sys_gui("entry $id.notes.low_c -textvariable $var_low_c -width 7\n");
    sys_gui("pack $id.notes.loctaves $id.notes.octaves $id.notes.llow_c $id.notes.low_c -side left\n");

    sys_gui("frame $id.checkframe\n");
    sys_gui("pack $id.checkframe -side top\n");
    sys_gui("checkbutton $id.checkframe.play -text \"Play with computer Keyboard?\"  -variable $var_keyb_play\n");
    sys_gui("pack $id.checkframe.play -side left\n");

    sys_gui("frame $id.buttonframe\n");
    sys_gui("pack $id.buttonframe -side bottom -fill x -pady 2m\n");
    sys_gui("button $id.buttonframe.cancel -text {Cancel} -command \"keyboard_cancel $id\"\n");
    sys_gui("button $id.buttonframe.apply -text {Apply} -command \"keyboard_apply $id\"\n");
    sys_gui("button $id.buttonframe.ok -text {OK} -command \"keyboard_ok $id\"\n");
    sys_gui("pack $id.buttonframe.cancel -side left -expand 1\n");
    sys_gui("pack $id.buttonframe.apply -side left -expand 1\n");
    sys_gui("pack $id.buttonframe.ok -side left -expand 1\n");

    sys_gui("focus $id.size.width\n");
    sys_gui("}\n");
// Apply function
    sys_gui("proc keyboard_apply {id} {\n");
    sys_gui("set vid [string trimleft $id .]\n");
    sys_gui("set var_width [concat var_width_$vid]\n");
    sys_gui("set var_height [concat var_height_$vid]\n");
    sys_gui("set var_octaves [concat var_octaves_$vid]\n");
    sys_gui("set var_low_c [concat var_low_c_$vid]\n");
    sys_gui("set var_keyb_play [concat var_keyb_play_$vid]\n");

    sys_gui("global $var_width\n");
    sys_gui("global $var_height\n");
    sys_gui("global $var_octaves\n");
    sys_gui("global $var_low_c\n");
    sys_gui("global $var_keyb_play\n");
// Aply function
    sys_gui("set cmd [concat $id apply \
        [eval concat $$var_width] \
        [eval concat $$var_height] \
        [eval concat $$var_octaves] \
        [eval concat $$var_low_c] \
        [eval concat $$var_keyb_play] \\;]\n");
    sys_gui("pd $cmd\n");
    sys_gui("}\n");
// Cancel function
    sys_gui("proc keyboard_cancel {id} {\n");
    sys_gui("set cmd [concat $id cancel \\;]\n");
    sys_gui("pd $cmd\n");
    sys_gui("}\n");
// OK function
    sys_gui("proc keyboard_ok {id} {\n");
    sys_gui("keyboard_apply $id\n");
    sys_gui("keyboard_cancel $id\n");
    sys_gui("}\n");
}
