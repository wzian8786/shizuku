(%module m1
    (%port %input p1)
    (%port %output p2)
    (%port %inout p3)
    (%net n1
        (%upport p1)
        (%upport p2)
    )
    (%net n2
        (%downport i1 p4)
        (%downport i2 p4)
        (%downport i3 p5)
    )
    (%hier \m2 \i1 )
    (%hier m2 \i2 )
    (%hier m3 i3)
    (%leaf i j)
    (%leaf k m)
)
(%module m2
    (%port %input p4)
)
(%module m3
    (%port %output p5)
)
(%module m4)
