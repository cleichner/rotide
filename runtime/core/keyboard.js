ro.cmd_history = [];

/*
 * Creates a binding for inserting characters.
 * DEFAULT: i
 */
ro.bind([ro.I], "insert", function(buffer) {
        if (!ro.insert_mode) ro.insert_mode = true;
        ro.status = "-- INSERT --"
});

/*
 * Creates a binding for leaving insert mode.
 * DEFAULT: esc
 */
ro.bind([ro.ESC], "command", function () {
    ro.status = "-- WAITING --";

    if (ro.insert_mode) {
        ro.insert_mode = false;
        return true;
    }

    // The reason why we return false is to let the
    // engine know that this key combination did not
    // do anything, so other key combinations can be
    // used with it, for example ESC->J or so on.
    return false
})

ro.on_command(function (cmd) {
    ro.cmd_history.append(cmd);
});
