.assembly extern mscorlib {}
.assembly DefaultName {}

.method static void Main()
{
    .entrypoint
    .maxstack 8
    .locals init (
        [0] int32 V_0,
        [1] int32 V_1
    )
 ldc.i4	5
 stloc	0
 ldloc	0
 ldc.i4	2
 add
 stloc	1
 ldloc	1
 call void class [mscorlib]System.Console::WriteLine(int32)
ret}
