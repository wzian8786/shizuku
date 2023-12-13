(%module m1
    (%port p1 %input)
    (%port p2 %output)
    (%port p3 %inout)
    (%net n1
        (%upport p1)
        (%upport p2)
    )
    (%net n2
        (%downport i1 p4)
        (%downport i2 p4)
        (%downport i3 p5)
        (%pport i4 p1)
        (%pport i4 p2)
    )
    (%net n3)
    (%hier \i1 \m2 )
    (%hier i2 m2)
    (%hier i3 m3)
    (%pinst i4 m4)
)
(%module m2
    (%port p4 %input)
)
(%module m3
    (%port p5 %output)
)
(%process m4 %seq
    (%port p1 %input)
    (%port p2 %input)
)
