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
    )
    (%net n3)
    (%hier \m2 \i1 )
    (%hier m2 \i2 )
    (%hier m3 i3)
    (%pinst i j)
    (%pinst k m)
)
(%module m2
    (%port p4 %input)
)
(%module m3
    (%port p5 %output)
)
(%process m4 %seq)
(%process m5 %call)
(%process m6 %comb)
