(%module m1
    (%net n1
        (%iport mi1 in)
        (%pport pi2 1)
    )
    (%net n2
        (%iport mi1 out1)
        (%pport pi1 1)
    )
    (%net n3
        (%iport mi1 out2)
        (%pport pi3 1)
    )
    (%net n4
        (%iport mi1 out3)
        (%pport pi4 1)
    )
    (%minst mi1 m2)
    (%pinst pi1 proc1)
    (%pinst pi2 proc2)
    (%pinst pi3 proc1)
    (%pinst pi4 proc1)
)
(%module m2
    (%port in %input)
    (%port out1 %output)
    (%port out2 %output)
    (%port out3 %output)
    (%net net1
        (%mport in)
        (%iport inst1 in)
    )
    (%net net2
        (%iport inst1 out)
        (%iport inst2 in)
    )
    (%net net3
        (%iport inst2 out)
        (%iport inst3 in)
    )
    (%net net4
        (%mport out1)
        (%mport out2)
        (%mport out3)
        (%iport inst3 out)
    )
    (%minst inst1 m3)
    (%minst inst2 m3)
    (%minst inst3 m3)
)
(%module m3
    (%port in %input)
    (%port out %output)
    (%net net1
        (%mport in)
        (%mport out)
        (%pport inst 1)
    )
    (%pinst inst proc1)
)
(%process proc1 %seq
    (%port %input)
)
(%process proc2 %seq
    (%port %output)
)
