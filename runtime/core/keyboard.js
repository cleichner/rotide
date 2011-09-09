ro.cmd_history = [];
ro.multiplier = "";

/**
 * Insert Mode
 */
ro.bind([ro.I], "insert", function(buffer) {
        if (!ro.insert_mode) ro.insert_mode = true;
        ro.status = "-- INSERT --"
        // Refocus cursor
        ro.mx = ro.mx;
        ro.my = ro.my;
});

/**
 * Command mode
 */
ro.bind([ro.COLON], "command_text", function () {
    if (ro.insert_mode) return false;
    ro.status = "#";
    ro.cmd_mode = true;
});

/**
 * Internal function. mx/my should be 1, 0, or -1.
 */
function move_cursor_delta(mx, my) {
    return function () {
        var inc = ro.multiplier.length ? parseInt(ro.multiplier) : 1;

        if (mx == 0) {
            ro.status = "" + (ro.my + my*inc) + ":" + ro.mx;
            ro.my += my*inc;
        } else if (my == 0) {
            ro.status = "" + ro.my + ":" + (ro.mx + mx*inc);
            ro.mx += mx*inc;
        } else {
            ro.status = "" + (ro.my + my*inc) + ":" + (ro.mx + mx*inc);
            ro.mx += mx*inc;
            ro.my += my*inc;
        }

         if (ro.multiplier.length)
            ro.multiplier = "";

         return true;
    }
};

/**
 * Cursor movement
 */
ro.bind([ro.J], "down", move_cursor_delta(0, 1));
ro.bind([ro.K], "up", move_cursor_delta(0, -1));
ro.bind([ro.L], "right", move_cursor_delta(1, 0));
ro.bind([ro.H], "left", move_cursor_delta(-1, 0));

/**
 * Special case of key 0
 */
ro.bind([ro.ZERO], "0", function (argument) {
    if (ro.insert_mode) return false;

    if (ro.multiplier.length) {
        ro.multiplier += "0";
    } else {
        ro.status = "" + ro.my + ":0";
        ro.mx = 0;
    }

    return true;
});

/**
 * Build number modifiers
 */
for (var i = 1; i < 10; i++) {
    ro.bind([ro.ZERO + i], "" + i, (function (n) {
        return function(arguments) {
            if (ro.insert_mode) return false;
            ro.multiplier += "" + n;
        };
    })(i));
}


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

    return false
})

/**
 * _ case
 */
ro.command(function (cmd, args) {
    return false;
});
