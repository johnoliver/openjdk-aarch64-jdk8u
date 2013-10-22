// BEGIN This section of the file is automatically generated. Do not edit --------------
define(`BASE_SHIFT_INSN',
`
instruct $2$1_reg_$4_reg(iReg$1NoSp dst,
                         iReg$1 src1, iReg$1 src2,
                         immI src3, rFlagsReg cr) %{
  match(Set dst ($2$1 src1 ($4$1 src2 src3)));

  ins_cost(DEFAULT_COST);
  format %{ "$3  $dst, $src1, $src2, $5 $src3" %}

  ins_encode %{
    __ $3(as_Register($dst$$reg),
              as_Register($src1$$reg),
              as_Register($src2$$reg),
              Assembler::$5,
              $src3$$constant & 0x3f);
  %}

  ins_pipe(pipe_class_default);
%}')
define(`BASE_INVERTED_INSN',
`
instruct $2$1_reg_not_reg(iReg$1NoSp dst,
                         iReg$1 src1, iReg$1 src2, imm$1_M1 m1,
                         rFlagsReg cr) %{
dnl This ifelse is because hotspot reassociates (xor (xor ..)..)
dnl into this canonical form.
  ifelse($2,Xor,
    match(Set dst (Xor$1 m1 (Xor$1 src2 src1)));,
    match(Set dst ($2$1 src1 (Xor$1 src2 m1)));)
  ins_cost(DEFAULT_COST);
  format %{ "$3  $dst, $src1, $src2" %}

  ins_encode %{
    __ $3(as_Register($dst$$reg),
              as_Register($src1$$reg),
              as_Register($src2$$reg),
              Assembler::LSL, 0);
  %}

  ins_pipe(pipe_class_default);
%}')
define(`INVERTED_SHIFT_INSN',
`
instruct $2$1_reg_$4_not_reg(iReg$1NoSp dst,
                         iReg$1 src1, iReg$1 src2,
                         immI src3, imm$1_M1 src4, rFlagsReg cr) %{
dnl This ifelse is because hotspot reassociates (xor (xor ..)..)
dnl into this canonical form.
  ifelse($2,Xor,
    match(Set dst ($2$1 src4 (Xor$1($4$1 src2 src3) src1)));,
    match(Set dst ($2$1 src1 (Xor$1($4$1 src2 src3) src4)));)
  ins_cost(DEFAULT_COST);
  format %{ "$3  $dst, $src1, $src2, $5 $src3" %}

  ins_encode %{
    __ $3(as_Register($dst$$reg),
              as_Register($src1$$reg),
              as_Register($src2$$reg),
              Assembler::$5,
              $src3$$constant & 0x3f);
  %}

  ins_pipe(pipe_class_default);
%}')
define(`NOT_INSN',
`instruct reg$1_not_reg(iReg$1NoSp dst,
                         iReg$1 src1, imm$1_M1 m1,
                         rFlagsReg cr) %{
  match(Set dst (Xor$1 src1 m1));
  ins_cost(DEFAULT_COST);
  format %{ "$2  $dst, $src1, zr" %}

  ins_encode %{
    __ $2(as_Register($dst$$reg),
              as_Register($src1$$reg),
              zr,
              Assembler::LSL, 0);
  %}

  ins_pipe(pipe_class_default);
%}')
dnl
define(`BOTH_SHIFT_INSNS',
`BASE_SHIFT_INSN(I, $1, ifelse($2,andr,andw,$2w), $3, $4)
BASE_SHIFT_INSN(L, $1, $2, $3, $4)')
dnl
define(`BOTH_INVERTED_INSNS',
`BASE_INVERTED_INSN(I, $1, $2, $3, $4)
BASE_INVERTED_INSN(L, $1, $2, $3, $4)')
dnl
define(`BOTH_INVERTED_SHIFT_INSNS',
`INVERTED_SHIFT_INSN(I, $1, $2w, $3, $4, ~0, int)
INVERTED_SHIFT_INSN(L, $1, $2, $3, $4, ~0l, long)')
dnl
define(`ALL_SHIFT_KINDS',
`BOTH_SHIFT_INSNS($1, $2, URShift, LSR)
BOTH_SHIFT_INSNS($1, $2, RShift, ASR)
BOTH_SHIFT_INSNS($1, $2, LShift, LSL)')
dnl
define(`ALL_INVERTED_SHIFT_KINDS',
`BOTH_INVERTED_SHIFT_INSNS($1, $2, URShift, LSR)
BOTH_INVERTED_SHIFT_INSNS($1, $2, RShift, ASR)
BOTH_INVERTED_SHIFT_INSNS($1, $2, LShift, LSL)')
dnl
NOT_INSN(L, eon)
NOT_INSN(I, eonw)
BOTH_INVERTED_INSNS(And, bic)
BOTH_INVERTED_INSNS(Or, orn)
BOTH_INVERTED_INSNS(Xor, eon)
ALL_INVERTED_SHIFT_KINDS(And, bic)
ALL_INVERTED_SHIFT_KINDS(Xor, eon)
ALL_INVERTED_SHIFT_KINDS(Or, orn)
ALL_SHIFT_KINDS(And, andr)
ALL_SHIFT_KINDS(Xor, eor)
ALL_SHIFT_KINDS(Or, orr)
ALL_SHIFT_KINDS(Add, add)
ALL_SHIFT_KINDS(Sub, sub)
dnl
dnl EXTEND mode, rshift_op, src, lshift_count, rshift_count
define(`EXTEND', `($2$1 (LShift$1 $3 $4) $5)')
define(`BFM_INSN',`
// Shift Left followed by Shift Right.
// This idiom is used by the compiler for the i2b bytecode etc.
instruct $4$1(iReg$1NoSp dst, iReg$1 src, immI lshift_count, immI rshift_count)
%{
  match(Set dst EXTEND($1, $3, src, lshift_count, rshift_count));
  // Make sure we are not going to exceed what $4 can do.
  predicate((unsigned int)n->in(2)->get_int() <= $2
            && (unsigned int)n->in(1)->in(2)->get_int() <= $2);

  format %{ "$4  $dst, $src, $rshift_count - $lshift_count, #$2 - $lshift_count" %}
  ins_encode %{
    int lshift = $lshift_count$$constant, rshift = $rshift_count$$constant;
    int s = $2 - lshift;
    int r = (rshift - lshift) & $2;
    __ $4(as_Register($dst$$reg),
	    as_Register($src$$reg),
	    r, s);
  %}

  ins_pipe(pipe_class_default);
%}')
BFM_INSN(L, 63, RShift, sbfm)
BFM_INSN(I, 31, RShift, sbfmw)
BFM_INSN(L, 63, URShift, ubfm)
BFM_INSN(I, 31, URShift, ubfmw)
dnl
// Bitfield extract with shift & mask
define(`BFX_INSN',
`instruct $3$1(iReg$1NoSp dst, iReg$1 src, immI rshift, imm$1_bitmask mask)
%{
  match(Set dst (And$1 ($2$1 src rshift) mask));

  format %{ "$3 $dst, $src, $mask" %}
  ins_encode %{
    int rshift = $rshift$$constant;
    long mask = $mask$$constant;
    int width = exact_log2(mask+1);
    __ $3(as_Register($dst$$reg),
	    as_Register($src$$reg), rshift, width);
  %}
  ins_pipe(pipe_class_default);
%}')
BFX_INSN(I,URShift,ubfxw)
BFX_INSN(L,URShift,ubfx)

// We can use ubfx when extending an And with a mask when we know mask
// is positive.  We know that because immI_bitmask guarantees it.
instruct ubfxIConvI2L(iRegLNoSp dst, iRegI src, immI rshift, immI_bitmask mask)
%{
  match(Set dst (ConvI2L (AndI (URShiftI src rshift) mask)));

  format %{ "ubfx $dst, $src, $mask" %}
  ins_encode %{
    int rshift = $rshift$$constant;
    long mask = $mask$$constant;
    int width = exact_log2(mask+1);
    __ ubfx(as_Register($dst$$reg),
	    as_Register($src$$reg), rshift, width);
  %}
  ins_pipe(pipe_class_default);
%}
dnl
// Rotations
define(`EXTRACT_INSN',
`instruct extr$3$1(iReg$1NoSp dst, iReg$1 src1, iReg$1 src2, immI lshift, immI rshift, rFlagsReg cr)
%{
  match(Set dst ($3$1 (LShift$1 src1 lshift) (URShift$1 src2 rshift)));
  predicate(0 == ((n->in(1)->in(2)->get_int() + n->in(2)->in(2)->get_int()) & $2));

  format %{ "extr $dst, $src1, $src2, #$rshift" %}

  ins_encode %{
    __ $4(as_Register($dst$$reg), as_Register($src1$$reg), as_Register($src2$$reg),
            $rshift$$constant & $2);
  %}
  ins_pipe(pipe_class_default);
%}
')
EXTRACT_INSN(L, 63, Or, extr)
EXTRACT_INSN(I, 31, Or, extrw)
EXTRACT_INSN(L, 63, Add, extr)
EXTRACT_INSN(I, 31, Add, extrw)
dnl
// Add/subtract (extended)
dnl ADD_SUB_EXTENDED(mode, size, add node, shift node, insn, shift type, wordsize
define(`ADD_SUB_CONV', `
instruct $3Ext$1(iReg$1NoSp dst, iReg$1 src1, iRegI src2, rFlagsReg cr)
%{
  match(Set dst ($3$1 src1 (ConvI2L src2)));
  format %{ "$4  $dst, $src1, $6 $src2" %}

   ins_encode %{
     __ $4(as_Register($dst$$reg), as_Register($src1$$reg),
            as_Register($src2$$reg), ext::$5);
   %}
  ins_pipe(pipe_class_default);
%}')
ADD_SUB_CONV(I,L,Add,add,sxtw);
ADD_SUB_CONV(I,L,Sub,sub,sxtw);
dnl
define(`ADD_SUB_EXTENDED', `
instruct $3Ext$1_$6(iReg$1NoSp dst, iReg$1 src1, iReg$1 src2, immI_`'eval($7-$2) lshift, immI_`'eval($7-$2) rshift, rFlagsReg cr)
%{
  match(Set dst ($3$1 src1 EXTEND($1, $4, src2, lshift, rshift)));
  format %{ "$5  $dst, $src1, $6 $src2" %}

   ins_encode %{
     __ $5(as_Register($dst$$reg), as_Register($src1$$reg),
            as_Register($src2$$reg), ext::$6);
   %}
  ins_pipe(pipe_class_default);
%}')
ADD_SUB_EXTENDED(I,16,Add,RShift,add,sxth,32)
ADD_SUB_EXTENDED(I,8,Add,RShift,add,sxtb,32)
ADD_SUB_EXTENDED(I,8,Add,URShift,add,uxtb,32)
ADD_SUB_EXTENDED(L,16,Add,RShift,add,sxth,64)
ADD_SUB_EXTENDED(L,32,Add,RShift,add,sxtw,64)
ADD_SUB_EXTENDED(L,8,Add,RShift,add,sxtb,64)
ADD_SUB_EXTENDED(L,8,Add,URShift,add,uxtb,64)
dnl
dnl ADD_SUB_ZERO_EXTEND(mode, size, add node, insn, shift type)
define(`ADD_SUB_ZERO_EXTEND', `
instruct $3Ext$1_$5_and(iReg$1NoSp dst, iReg$1 src1, iReg$1 src2, imm$1_$2 mask, rFlagsReg cr)
%{
  match(Set dst ($3$1 src1 (And$1 src2 mask)));
  format %{ "$4  $dst, $src1, $src2, $5" %}

   ins_encode %{
     __ $4(as_Register($dst$$reg), as_Register($src1$$reg),
            as_Register($src2$$reg), ext::$5);
   %}
  ins_pipe(pipe_class_default);
%}')
dnl
ADD_SUB_ZERO_EXTEND(I,255,Add,addw,uxtb)
ADD_SUB_ZERO_EXTEND(I,65535,Add,addw,uxth)
ADD_SUB_ZERO_EXTEND(L,255,Add,add,uxtb)
ADD_SUB_ZERO_EXTEND(L,65535,Add,add,uxth)
ADD_SUB_ZERO_EXTEND(L,4294967295,Add,add,uxtw)
dnl
ADD_SUB_ZERO_EXTEND(I,255,Sub,subw,uxtb)
ADD_SUB_ZERO_EXTEND(I,65535,Sub,subw,uxth)
ADD_SUB_ZERO_EXTEND(L,255,Sub,sub,uxtb)
ADD_SUB_ZERO_EXTEND(L,65535,Sub,sub,uxth)
ADD_SUB_ZERO_EXTEND(L,4294967295,Sub,sub,uxtw)
// END This section of the file is automatically generated. Do not edit --------------
