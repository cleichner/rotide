ro.cmd_history = [];

/*
 * Creates a binding for inserting characters.
 * DEFAULT: i
 */
ro.bind([ro.I], "insert", function(buffer) {
        if (!ro.insert_mode) ro.insert_mode = true;
        ro.status = "-- INSERT --"
});

ro.bind([ro.CTRL_A, ro.CTRL_S], "test", function (args) {
    for (i = 0; i < args.length; i++)
        ro.test(10 + i, 5, "Whoa, you said " + args[i] + " which is crazy.");
});

ro.bind([ro.COLON], "command_text", function () {
    if (ro.insert_mode) return false;
    ro.cmd_mode = true;
});

/*
 * Creates a binding for leaving insert mode.
 * DEFAULT: esc
 */
ro.bind([ro.ESC], "command", function () {
    ro.status = "-- WAITING --";
    ro.cmd_mode = false;

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

ro.command(function (cmd) {
    ro.status = "-- BAD! --";
});
