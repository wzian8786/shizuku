(%module m1
    (%net n1
        (%iport mi1 p1)
    )
    (%net n2
        (%pport pi1 2)
    )
    (%minst mi1 m2)
    (%minst mi2 m3)
    (%pinst pi1 proc1)
)
(%module m2
    (%port p1 %input)
    (%port p2 %input)
)
(%module m3
    (%port p3 %output)
)
(%process proc1 %seq
    (%port %input)
    (%port %input)
)
