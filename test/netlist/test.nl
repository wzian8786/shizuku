(%module m1
    (%port p1 %input)
    (%port p2 %output)
    (%port p3 %inout)
    (%net n1
        (%mport p1)
        (%mport p2)
    )
    (%net n2
        (%iport i1 p4)
        (%iport i2 p4)
        (%iport i3 p5)
        (%pport i4 0)
        (%pport i4 1)
    )
    (%net n3)
    (%minst \i1 \m2 )
    (%minst i2 m2)
    (%minst i3 m3)
    (%pinst i4 m4)
)
(%module m2
    (%port p4 %input)
)
(%module m3
    (%port p5 %output)
)
(%process m4 %seq
    (%port %input)
    (%port %input)
)
