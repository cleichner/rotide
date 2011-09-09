ro.cmd_history = [];

/*
 * Creates a binding for inserting characters.
 * DEFAULT: i
 */
ro.bind([ro.I], "insert", function(buffer) {
        if (!ro.insert_mode) ro.insert_mode = true;
        ro.status = "-- INSERT --"
        // Refocus cursor
        ro.mx = ro.mx;
        ro.my = ro.my;
});

ro.bind([ro.COLON], "command_text", function () {
    if (ro.insert_mode) return false;
    ro.status = "#";
    ro.cmd_mode = true;
});

ro.bind([106], "down", function (argument) {
    if (ro.insert_mode) return false;
    ro.my += 1;
    return true;
});

ro.bind([107], "up", function (argument) {
    if (ro.insert_mode) return false;
    ro.my -= 1;
    return true;
});

ro.bind([108], "right", function (argument) {
    if (ro.insert_mode) return false;
    ro.mx += 1;
    return true;
});

ro.bind([104], "left", function (argument) {
    if (ro.insert_mode) return false;
    ro.mx -= 1;
    return true;
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

ro.command(function (cmd, args) {
    if (cmd == "hello") {
        ro.test(3, 2, "Interepreted it correctly!");
        return true;
    }

    return false;
});
