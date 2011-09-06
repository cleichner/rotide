var i = 0;
ro.bind([ro.I], function(buffer) {
        if (!ro.insert_mode) ro.insert_mode = true;
        ro.status = "-- INSERT --"
        //ro.test(10, 10, "Called " + (i++) + " times.");
        //buffer.lines[buffer.current_line].append('foo');
});

ro.bind([ro.ESC], function () {
    ro.status = "-- WAITING --";
    if (ro.insert_mode) {
        ro.insert_mode = false;
        return true;
    } else {
        return false;
    }
})
