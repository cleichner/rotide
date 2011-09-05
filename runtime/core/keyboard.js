ro
    .bind("j", function(buffer) {
        ro.test(10, 10, "Hello!");
        if (!ro.insert_mode)
            return;
        //buffer.lines[buffer.current_line].append('foo');
    });
