if {[catch {pd}] } {
    proc pd {args} {pdsend [join $args " "]}
}

proc keyboard_mousepress {id x y b} {
    if {$b == 1} {
        pd [concat keyboard _mousepress $x $y $id\;]
    }
}

proc keyboard_mouserelease {id x y b} {
    if {$b == 1} {
        pd [concat keyboard _mouserelease $x $y $id\;]
    }
}

proc keyboard_mousemotion {id x y} {
    set cmd [concat keyboard _mousemotion $x $y $id\;]
    pd $cmd
}

proc keyboard_keydown {id K} {
    pd [concat keyboard _kdown $K \;]
}

proc keyboard_keyup {id K} {
    pd [concat keyboard _kup $K \;]
}

proc keyboard_ok {id} {
    keyboard_apply $id
    keyboard_cancel $id
}

proc keyboard_cancel {id} {
    set cmd [concat $id cancel \;]
    pd $cmd
}

proc keyboard_apply {id} {
    set vid [string trimleft $id .]
    set var_width [concat var_width_$vid]
    set var_height [concat var_height_$vid]
    set var_octaves [concat var_octaves_$vid]
    set var_low_c [concat var_low_c_$vid]
    set var_keyb_play [concat var_keyb_play_$vid]

    global $var_width
    global $var_height
    global $var_octaves
    global $var_low_c
    global $var_keyb_play

    set cmd [concat $id apply \
        [eval concat $$var_width] \
        [eval concat $$var_height] \
        [eval concat $$var_octaves] \
        [eval concat $$var_low_c] \
        [eval concat $$var_keyb_play] \;]
    pd $cmd
}

proc keyboard_properties {id width height octaves low_c keyb_play} {
    set vid [string trimleft $id .]
    set var_width [concat var_width_$vid]
    set var_height [concat var_height_$vid]
    set var_octaves [concat var_octaves_$vid]
    set var_low_c [concat var_low_c_$vid]
    set var_keyb_play [concat var_keyb_play_$vid]

    global $var_width
    global $var_height
    global $var_octaves
    global $var_low_c
    global $var_keyb_play

    set $var_width $width
    set $var_height $height
    set $var_octaves $octaves
    set $var_low_c $low_c
    set $var_keyb_play $keyb_play

    toplevel $id
    wm title $id {Keyboard}
    wm protocol $id WM_DELETE_WINDOW [concat keyboard_cancel $id]

    label $id.label -text {Keyboard}
    pack $id.label -side top

    frame $id.size
    pack $id.size -side top
    label $id.size.lwidth -text "Key Width:"
    entry $id.size.width -textvariable $var_width -width 7
    label $id.size.lheight -text "Height:"
    entry $id.size.height -textvariable $var_height -width 7
    pack $id.size.lwidth $id.size.width $id.size.lheight $id.size.height -side left

    frame $id.notes
    pack $id.notes -side top
    label $id.notes.loctaves -text "Octaves:"
    entry $id.notes.octaves -textvariable $var_octaves -width 7
    label $id.notes.llow_c -text "Low C:"
    entry $id.notes.low_c -textvariable $var_low_c -width 7
    pack $id.notes.loctaves $id.notes.octaves $id.notes.llow_c $id.notes.low_c -side left

    frame $id.checkframe
    pack $id.checkframe -side top
    checkbutton $id.checkframe.play -text "Play with computer Keyboard?"  -variable $var_keyb_play
    pack $id.checkframe.play -side left

    frame $id.buttonframe
    pack $id.buttonframe -side bottom -fill x -pady 2m
    button $id.buttonframe.cancel -text {Cancel} -command "keyboard_cancel $id"
    button $id.buttonframe.apply -text {Apply} -command "keyboard_apply $id"
    button $id.buttonframe.ok -text {OK} -command "keyboard_ok $id"
    pack $id.buttonframe.cancel -side left -expand 1
    pack $id.buttonframe.apply -side left -expand 1
    pack $id.buttonframe.ok -side left -expand 1

    focus $id.size.width
}
