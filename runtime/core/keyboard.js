ro
    .require(ro.insert_mode)
    .bind('j', function(buffer) {
        buffer.line.append('foo');
    });

            
