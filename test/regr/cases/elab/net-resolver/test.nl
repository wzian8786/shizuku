(%module m1
    (%port p1 %input)
    (%net n1
        (%mport p1)
        (%iport mi1 p1)
        (%pport pi1 2)
    )
    (%minst mi1 m2)
    (%pinst pi1 proc1)
)
(%module m2
    (%port p1 %output)
)
(%process proc1 %seq
    (%port %input)
    (%port %output)
)
