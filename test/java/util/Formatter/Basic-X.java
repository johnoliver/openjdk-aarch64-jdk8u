/*
 * Copyright 2003-2007 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 */

/* Type-specific source code for unit test
 *
 * Regenerate the BasicX classes via genBasic.sh whenever this file changes.
 * We check in the generated source files so that the test tree can be used
 * independently of the rest of the source tree.
 */

#warn This file is preprocessed before being compiled

import java.io.*;
import java.math.BigDecimal;
import java.math.BigInteger;
import java.text.DateFormatSymbols;
import java.util.*;
#if[double]
import sun.misc.FpUtils;
import sun.misc.DoubleConsts;
#end[double]

import static java.util.Calendar.*;
#if[datetime]
import static java.util.SimpleTimeZone.*;
import java.util.regex.Pattern;
#end[datetime]

public class Basic$Type$ extends Basic {

    private static void test(String fs, String exp, Object ... args) {
        Formatter f = new Formatter(new StringBuilder(), Locale.US);
        f.format(fs, args);
        ck(fs, exp, f.toString());
    }

    private static void test(Locale l, String fs, String exp, Object ... args)
    {
        Formatter f = new Formatter(new StringBuilder(), l);
        f.format(fs, args);
        ck(fs, exp, f.toString());
    }

    private static void test(String fs, Object ... args) {
        Formatter f = new Formatter(new StringBuilder(), Locale.US);
        f.format(fs, args);
        ck(fs, "fail", f.toString());
    }

    private static void test(String fs) {
        Formatter f = new Formatter(new StringBuilder(), Locale.US);
        f.format(fs, "fail");
        ck(fs, "fail", f.toString());
    }

    private static void testSysOut(String fs, String exp, Object ... args) {
        FileOutputStream fos = null;
        FileInputStream fis = null;
        try {
            PrintStream saveOut = System.out;
            fos = new FileOutputStream("testSysOut");
            System.setOut(new PrintStream(fos));
            System.out.format(Locale.US, fs, args);
            fos.close();

            fis = new FileInputStream("testSysOut");
            byte [] ba = new byte[exp.length()];
            int len = fis.read(ba);
            String got = new String(ba);
            if (len != ba.length)
                fail(fs, exp, got);
            ck(fs, exp, got);

            System.setOut(saveOut);
        } catch (FileNotFoundException ex) {
            fail(fs, ex.getClass());
        } catch (IOException ex) {
            fail(fs, ex.getClass());
        } finally {
            try {
                if (fos != null)
                    fos.close();
                if (fis != null)
                    fis.close();
            } catch (IOException ex) {
                fail(fs, ex.getClass());
            }
        }
    }

    private static void tryCatch(String fs, Class<?> ex) {
        boolean caught = false;
        try {
            test(fs);
        } catch (Throwable x) {
            if (ex.isAssignableFrom(x.getClass()))
                caught = true;
        }
        if (!caught)
            fail(fs, ex);
        else
            pass();
    }

    private static void tryCatch(String fs, Class<?> ex, Object ... args) {
        boolean caught = false;
        try {
            test(fs, args);
        } catch (Throwable x) {
            if (ex.isAssignableFrom(x.getClass()))
                caught = true;
        }
        if (!caught)
            fail(fs, ex);
        else
            pass();
    }

#if[datetime]
    private static void testDateTime(String fs, String exp, Calendar c) {
        testDateTime(fs, exp, c, true);
    }

    private static void testDateTime(String fs, String exp, Calendar c, boolean upper) {
        //---------------------------------------------------------------------
        // Date/Time conversions applicable to Calendar, Date, and long.
        //---------------------------------------------------------------------

        // Calendar
        test(fs, exp, c);
        test((Locale)null, fs, exp, c);
        test(Locale.US, fs, exp, c);

        // Date/long do not have timezone information so they will always use
        // the default timezone.
        String nexp = (fs.equals("%tZ") || fs.equals("%TZ")
                       || fs.equals("%tc") || fs.equals("%Tc")
                       ? exp.replace("PST", "GMT-08:00")
                       : exp);

        // Date (implemented via conversion to Calendar)
        Date d = c.getTime();
        test(fs, nexp, d);
        test((Locale)null, fs, nexp, d);
        test(Locale.US, fs, nexp, d);

        // long (implemented via conversion to Calendar)
        long l = c.getTimeInMillis();
        test(fs, nexp, l);
        test((Locale)null, fs, nexp, l);
        test(Locale.US, fs, nexp, l);

        if (upper)
            // repeat all tests for upper case variant (%T)
            testDateTime(Pattern.compile("t").matcher(fs).replaceFirst("T"),
                         exp.toUpperCase(), c, false);
    }

    private static void testHours() {
        for (int i = 0; i < 24; i++) {
            // GregorianCalendar(int year, int month, int dayOfMonth,
            //    int hourOfDay, int minute, int second);
            Calendar c = new GregorianCalendar(1995, MAY, 23, i, 48, 34);

            //-----------------------------------------------------------------
            // DateTime.HOUR_OF_DAY - 'k' (0 - 23) -- like H
            //-----------------------------------------------------------------
            String exp = Integer.toString(i);
            testDateTime("%tk", exp, c);

            //-----------------------------------------------------------------
            // DateTime.HOUR - 'l' (1 - 12) -- like I
            //-----------------------------------------------------------------
            int v = i % 12;
            v = (v == 0 ? 12 : v);
            String exp2 = Integer.toString(v);
            testDateTime("%tl", exp2, c);

            //-----------------------------------------------------------------
            // DateTime.HOUR_OF_DAY_0 - 'H' (00 - 23) [zero padded]
            //-----------------------------------------------------------------
            if (exp.length() < 2) exp = "0" + exp;
            testDateTime("%tH", exp, c);

            //-----------------------------------------------------------------
            // DateTime.HOUR_0 - 'I' (01 - 12)
            //-----------------------------------------------------------------
            if (exp2.length() < 2) exp2 = "0" + exp2;
            testDateTime("%tI", exp2, c);

            //-----------------------------------------------------------------
            // DateTime.AM_PM - (am or pm)
            //-----------------------------------------------------------------
            testDateTime("%tp", (i <12 ? "am" : "pm"), c);
        }
    }
#end[datetime]

#if[dec]
#if[prim]
    private static $type$ negate($type$ v) {
        return ($type$) -v;
    }
#end[prim]
#end[dec]
#if[Byte]
    private static $type$ negate($type$ v) {
        return new $type$((byte) -v.byteValue());
    }
#end[Byte]
#if[Short]
    private static $type$ negate($type$ v) {
        return new $type$((short) -v.shortValue());
    }
#end[Short]
#if[Integer]
    private static $type$ negate($type$ v) {
        return new $type$(-v.intValue());
    }
#end[Integer]
#if[Long]
    private static $type$ negate($type$ v) {
        return new $type$(-v.longValue());
    }
#end[Long]

#if[BigDecimal]
    private static $type$ create(double v) {
        return new $type$(v);
    }

    private static $type$ negate($type$ v) {
        return v.negate();
    }

    private static $type$ mult($type$ v, double mul) {
        return v.multiply(new $type$(mul));
    }

    private static $type$ recip($type$ v) {
        return BigDecimal.ONE.divide(v);
    }
#end[BigDecimal]
#if[float]
    private static $type$ create(double v) {
        return ($type$) v;
    }

    private static $type$ negate(double v) {
        return ($type$) -v;
    }

    private static $type$ mult($type$ v, double mul) {
        return v * ($type$) mul;
    }

    private static $type$ recip($type$ v) {
        return 1.0f / v;
    }
#end[float]
#if[Float]
    private static $type$ create(double v) {
        return new $type$(v);
    }

    private static $type$ negate($type$ v) {
        return new $type$(-v.floatValue());
    }

    private static $type$ mult($type$ v, double mul) {
        return new $type$(v.floatValue() * (float) mul);
    }

    private static $type$ recip($type$ v) {
        return new $type$(1.0f / v.floatValue());
    }
#end[Float]
#if[double]
    private static $type$ create(double v) {
        return ($type$) v;
    }


    private static $type$ negate(double v) {
        return -v;
    }

    private static $type$ mult($type$ v, double mul) {
        return v * mul;
    }

    private static $type$ recip($type$ v) {
        return 1.0 / v;
    }
#end[double]
#if[Double]
    private static $type$ create(double v) {
        return new $type$(v);
    }

    private static $type$ negate($type$ v) {
        return new $type$(-v.doubleValue());
    }

    private static $type$ mult($type$ v, double mul) {
        return new $type$(v.doubleValue() * mul);
    }

    private static $type$ recip($type$ v) {
        return new $type$(1.0 / v.doubleValue());
    }
#end[Double]

    public static void test() {
        TimeZone.setDefault(TimeZone.getTimeZone("GMT-0800"));

        // Any characters not explicitly defined as conversions, date/time
        // conversion suffixes, or flags are illegal and are reserved for
        // future extensions.  Use of such a character in a format string will
        // cause an UnknownFormatConversionException or
        // UnknownFormatFlagsException to be thrown.
        tryCatch("%q", UnknownFormatConversionException.class);
        tryCatch("%t&", UnknownFormatConversionException.class);
        tryCatch("%&d", UnknownFormatConversionException.class);
        tryCatch("%^b", UnknownFormatConversionException.class);

        //---------------------------------------------------------------------
        // Formatter.java class javadoc examples
        //---------------------------------------------------------------------
        test(Locale.FRANCE, "e = %+10.4f", "e =    +2,7183", Math.E);
        test("%4$2s %3$2s %2$2s %1$2s", " d  c  b  a", "a", "b", "c", "d");
        test("Amount gained or lost since last statement: $ %,(.2f",
             "Amount gained or lost since last statement: $ (6,217.58)",
             (new BigDecimal("-6217.58")));
        Calendar c = new GregorianCalendar(1969, JULY, 20, 16, 17, 0);
        testSysOut("Local time: %tT", "Local time: 16:17:00", c);

        test("Unable to open file '%1$s': %2$s",
             "Unable to open file 'food': No such file or directory",
             "food", "No such file or directory");
        Calendar duke = new GregorianCalendar(1995, MAY, 23, 19, 48, 34);
        duke.set(Calendar.MILLISECOND, 584);
        test("Duke's Birthday: %1$tB %1$te, %1$tY",
             "Duke's Birthday: May 23, 1995",
             duke);
        test("Duke's Birthday: %1$tB %1$te, %1$tY",
             "Duke's Birthday: May 23, 1995",
             duke.getTime());
        test("Duke's Birthday: %1$tB %1$te, %1$tY",
             "Duke's Birthday: May 23, 1995",
             duke.getTimeInMillis());

        test("%4$s %3$s %2$s %1$s %4$s %3$s %2$s %1$s",
             "d c b a d c b a", "a", "b", "c", "d");
        test("%s %s %<s %<s", "a b b b", "a", "b", "c", "d");
        test("%s %s %s %s", "a b c d", "a", "b", "c", "d");
        test("%2$s %s %<s %s", "b a a b", "a", "b", "c", "d");

        //---------------------------------------------------------------------
        // %b
        //
        // General conversion applicable to any argument.
        //---------------------------------------------------------------------
        test("%b", "true", true);
        test("%b", "false", false);
        test("%B", "TRUE", true);
        test("%B", "FALSE", false);
        test("%b", "true", Boolean.TRUE);
        test("%b", "false", Boolean.FALSE);
        test("%B", "TRUE", Boolean.TRUE);
        test("%B", "FALSE", Boolean.FALSE);
        test("%14b", "          true", true);
        test("%-14b", "true          ", true);
        test("%5.1b", "    f", false);
        test("%-5.1b", "f    ", false);

        test("%b", "true", "foo");
        test("%b", "false", (Object)null);

        // Boolean.java hardcodes the Strings for "true" and "false", so no
        // localization is possible.
        test(Locale.FRANCE, "%b", "true", true);
        test(Locale.FRANCE, "%b", "false", false);

        // If you pass in a single array to a varargs method, the compiler
        // uses it as the array of arguments rather than treating it as a
        // single array-type argument.
        test("%b", "false", (Object[])new String[2]);
        test("%b", "true", new String[2], new String[2]);

        int [] ia = { 1, 2, 3 };
        test("%b", "true", ia);

        //---------------------------------------------------------------------
        // %b - errors
        //---------------------------------------------------------------------
        tryCatch("%#b", FormatFlagsConversionMismatchException.class);
        tryCatch("%-b", MissingFormatWidthException.class);
        // correct or side-effect of implementation?
        tryCatch("%.b", UnknownFormatConversionException.class);
        tryCatch("%,b", FormatFlagsConversionMismatchException.class);

        //---------------------------------------------------------------------
        // %c
        //
        // General conversion applicable to any argument.
        //---------------------------------------------------------------------
        test("%c", "i", 'i');
        test("%C", "I", 'i');
        test("%4c",  "   i", 'i');
        test("%-4c", "i   ", 'i');
        test("%4C",  "   I", 'i');
        test("%-4C", "I   ", 'i');
        test("%c", "i", new Character('i'));
        test("%c", "H", (byte) 72);
        test("%c", "i", (short) 105);
        test("%c", "!", (int) 33);
        test("%c", "\u007F", Byte.MAX_VALUE);
        test("%c", new String(Character.toChars(Short.MAX_VALUE)),
             Short.MAX_VALUE);
        test("%c", "null", (Object) null);

        //---------------------------------------------------------------------
        // %c - errors
        //---------------------------------------------------------------------
        tryCatch("%c", IllegalFormatConversionException.class,
                 Boolean.TRUE);
        tryCatch("%c", IllegalFormatConversionException.class,
                 (float) 0.1);
        tryCatch("%c", IllegalFormatConversionException.class,
                 new Object());
        tryCatch("%c", IllegalFormatCodePointException.class,
                 Byte.MIN_VALUE);
        tryCatch("%c", IllegalFormatCodePointException.class,
                 Short.MIN_VALUE);
        tryCatch("%c", IllegalFormatCodePointException.class,
                 Integer.MIN_VALUE);
        tryCatch("%c", IllegalFormatCodePointException.class,
                 Integer.MAX_VALUE);

        tryCatch("%#c", FormatFlagsConversionMismatchException.class);
        tryCatch("%,c", FormatFlagsConversionMismatchException.class);
        tryCatch("%(c", FormatFlagsConversionMismatchException.class);
        tryCatch("%$c", UnknownFormatConversionException.class);
        tryCatch("%.2c", IllegalFormatPrecisionException.class);

        //---------------------------------------------------------------------
        // %s
        //
        // General conversion applicable to any argument.
        //---------------------------------------------------------------------
        test("%s", "Hello, Duke", "Hello, Duke");
        test("%S", "HELLO, DUKE", "Hello, Duke");
        test("%20S", "         HELLO, DUKE", "Hello, Duke");
        test("%20s", "         Hello, Duke", "Hello, Duke");
        test("%-20s", "Hello, Duke         ", "Hello, Duke");
        test("%-20.5s", "Hello               ", "Hello, Duke");
        test("%s", "null", (Object)null);

        StringBuffer sb = new StringBuffer("foo bar");
        test("%s", sb.toString(), sb);
        test("%S", sb.toString().toUpperCase(), sb);

        //---------------------------------------------------------------------
        // %s - errors
        //---------------------------------------------------------------------
        tryCatch("%-s", MissingFormatWidthException.class);
        tryCatch("%--s", DuplicateFormatFlagsException.class);
        tryCatch("%#s", FormatFlagsConversionMismatchException.class, 0);
        tryCatch("%#s", FormatFlagsConversionMismatchException.class, 0.5f);
        tryCatch("%#s", FormatFlagsConversionMismatchException.class, "hello");
        tryCatch("%#s", FormatFlagsConversionMismatchException.class, null);

        //---------------------------------------------------------------------
        // %h
        //
        // General conversion applicable to any argument.
        //---------------------------------------------------------------------
        test("%h", Integer.toHexString("Hello, Duke".hashCode()),
             "Hello, Duke");
        test("%10h", "  ddf63471", "Hello, Duke");
        test("%-10h", "ddf63471  ", "Hello, Duke");
        test("%-10H", "DDF63471  ", "Hello, Duke");
        test("%10h", "  402e0000", 15.0);
        test("%10H", "  402E0000", 15.0);

        //---------------------------------------------------------------------
        // %h - errors
        //---------------------------------------------------------------------
        tryCatch("%#h", FormatFlagsConversionMismatchException.class);

        //---------------------------------------------------------------------
        // flag/conversion errors
        //---------------------------------------------------------------------
        tryCatch("%F", UnknownFormatConversionException.class);

        tryCatch("%#g", FormatFlagsConversionMismatchException.class);

#if[dec]

#if[prim]
        $type$ minByte = Byte.MIN_VALUE;   // -128
#else[prim]
        $type$ minByte = new $type$(Byte.MIN_VALUE);
#end[prim]

        //---------------------------------------------------------------------
        // %d
        //
        // Numeric conversion applicable to byte, short, int, long, and
        // BigInteger.
        //---------------------------------------------------------------------
        test("%d", "null", (Object)null);

#if[byte]
#if[prim]
        //---------------------------------------------------------------------
        // %d - byte
        //---------------------------------------------------------------------
        $type$ seventeen = ($type$) 17;
        test("%d", "17", seventeen);
        test("%,d", "17", seventeen);
        test("%,d", "-17", negate(seventeen));
        test("%(d", "17", seventeen);
        test("%(d", "(17)", negate(seventeen));
        test("% d", " 17", seventeen);
        test("% d", "-17", negate(seventeen));
        test("%+d", "+17", seventeen);
        test("%+d", "-17", negate(seventeen));
        test("%010d", "0000000017", seventeen);
        test("%010d", "-000000017", negate(seventeen));
        test("%(10d", "      (17)", negate(seventeen));
        test("%-10d", "17        ", seventeen);
        test("%-10d", "-17       ", negate(seventeen));
#end[prim]
#else[byte]
#if[short]
        //---------------------------------------------------------------------
        // %d - short
        //---------------------------------------------------------------------
        $type$ oneToFive = ($type$) 12345;
        test("%d", "12345", oneToFive);
        test("%,d", "12,345", oneToFive);
        test("%,d", "-12,345", negate(oneToFive));
        test("%(d", "12345", oneToFive);
        test("%(d", "(12345)", negate(oneToFive));
        test("% d", " 12345", oneToFive);
        test("% d", "-12345", negate(oneToFive));
        test("%+d", "+12345", oneToFive);
        test("%+d", "-12345", negate(oneToFive));
        test("%010d", "0000012345", oneToFive);
        test("%010d", "-000012345", negate(oneToFive));
        test("%(10d", "   (12345)", negate(oneToFive));
        test("%-10d", "12345     ", oneToFive);
        test("%-10d", "-12345    ", negate(oneToFive));

#else[short]
#if[prim]
        //---------------------------------------------------------------------
        // %d - int and long
        //---------------------------------------------------------------------
        $type$ oneToSeven = ($type$) 1234567;
        test("%d", "1234567", oneToSeven);
        test("%,d", "1,234,567", oneToSeven);
        test(Locale.FRANCE, "%,d", "1\u00a0234\u00a0567", oneToSeven);
        test("%,d", "-1,234,567", negate(oneToSeven));
        test("%(d", "1234567", oneToSeven);
        test("%(d", "(1234567)", negate(oneToSeven));
        test("% d", " 1234567", oneToSeven);
        test("% d", "-1234567", negate(oneToSeven));
        test("%+d", "+1234567", oneToSeven);
        test("%+d", "-1234567", negate(oneToSeven));
        test("%010d", "0001234567", oneToSeven);
        test("%010d", "-001234567", negate(oneToSeven));
        test("%(10d", " (1234567)", negate(oneToSeven));
        test("%-10d", "1234567   ", oneToSeven);
        test("%-10d", "-1234567  ", negate(oneToSeven));
#end[prim]
#end[short]
#end[byte]
        //---------------------------------------------------------------------
        // %d - errors
        //---------------------------------------------------------------------
        tryCatch("%#d", FormatFlagsConversionMismatchException.class);
        tryCatch("%D", UnknownFormatConversionException.class);
        tryCatch("%0d", MissingFormatWidthException.class);
        tryCatch("%-d", MissingFormatWidthException.class);
        tryCatch("%7.3d", IllegalFormatPrecisionException.class);

        //---------------------------------------------------------------------
        // %o
        //
        // Numeric conversion applicable to byte, short, int, long, and
        // BigInteger.
        //---------------------------------------------------------------------
        test("%o", "null", (Object)null);

#if[byte]
        //---------------------------------------------------------------------
        // %o - byte
        //---------------------------------------------------------------------
        test("%010o", "0000000200", minByte);
        test("%-10o", "200       ", minByte);
        test("%#10o", "      0200", minByte);
#end[byte]
#if[short]
        //---------------------------------------------------------------------
        // %o - short
        //---------------------------------------------------------------------

        test("%010o", "0000177600", minByte);
        test("%-10o", "177600    ", minByte);
        test("%#10o", "   0177600", minByte);
#end[short]
#if[int]
        //---------------------------------------------------------------------
        // %o - int
        //---------------------------------------------------------------------
        test("%014o", "00037777777600", minByte);
        test("%-14o", "37777777600   ", minByte);
        test("%#14o", "  037777777600", minByte);

        $type$ oneToSevenOct = ($type$) 1234567;
        test("%o", "4553207", oneToSevenOct);
        test("%010o", "0004553207", oneToSevenOct);
        test("%-10o", "4553207   ", oneToSevenOct);
        test("%#10o", "  04553207", oneToSevenOct);
#end[int]
#if[long]
        //---------------------------------------------------------------------
        // %o - long
        //---------------------------------------------------------------------
        test("%024o", "001777777777777777777600", minByte);
        test("%-24o", "1777777777777777777600  ", minByte);
        test("%#24o", " 01777777777777777777600", minByte);

        $type$ oneToSevenOct = ($type$) 1234567;
        test("%o", "4553207", oneToSevenOct);
        test("%010o", "0004553207", oneToSevenOct);
        test("%-10o", "4553207   ", oneToSevenOct);
        test("%#10o", "  04553207", oneToSevenOct);
#end[long]

        //---------------------------------------------------------------------
        // %o - errors
        //---------------------------------------------------------------------
        tryCatch("%(o", FormatFlagsConversionMismatchException.class,
                 minByte);
        tryCatch("%+o", FormatFlagsConversionMismatchException.class,
                 minByte);
        tryCatch("% o", FormatFlagsConversionMismatchException.class,
                 minByte);
        tryCatch("%0o", MissingFormatWidthException.class);
        tryCatch("%-o", MissingFormatWidthException.class);
        tryCatch("%,o", FormatFlagsConversionMismatchException.class);
        tryCatch("%O", UnknownFormatConversionException.class);

        //---------------------------------------------------------------------
        // %x
        //
        // Numeric conversion applicable to byte, short, int, long, and
        // BigInteger.
        //---------------------------------------------------------------------
        test("%x", "null", (Object)null);

#if[byte]
        //---------------------------------------------------------------------
        // %x - byte
        //---------------------------------------------------------------------
        test("%010x", "0000000080", minByte);
        test("%-10x", "80        ", minByte);
        test("%#10x", "      0x80", minByte);
        test("%0#10x","0x00000080", minByte);
        test("%#10X", "      0X80", minByte);
        test("%X", "80", minByte);
#end[byte]
#if[short]
        //---------------------------------------------------------------------
        // %x - short
        //---------------------------------------------------------------------
        test("%010x", "000000ff80", minByte);
        test("%-10x", "ff80      ", minByte);
        test("%#10x", "    0xff80", minByte);
        test("%0#10x","0x0000ff80", minByte);
        test("%#10X", "    0XFF80", minByte);
        test("%X", "FF80", minByte);
#end[short]
#if[int]
        //---------------------------------------------------------------------
        // %x - int
        //---------------------------------------------------------------------
        $type$ oneToSevenHex = ($type$)1234567;
        test("%x", "null", (Object)null);
        test("%x", "12d687", oneToSevenHex);
        test("%010x", "000012d687", oneToSevenHex);
        test("%-10x", "12d687    ", oneToSevenHex);
        test("%#10x", "  0x12d687", oneToSevenHex);
        test("%#10X", "  0X12D687",oneToSevenHex);
        test("%X", "12D687", oneToSevenHex);

        test("%010x", "00ffffff80", minByte);
        test("%-10x", "ffffff80  ", minByte);
        test("%#10x", "0xffffff80", minByte);
        test("%0#12x","0x00ffffff80", minByte);
        test("%#12X", "  0XFFFFFF80", minByte);
        test("%X", "FFFFFF80", minByte);
#end[int]
#if[long]
        //---------------------------------------------------------------------
        // %x - long
        //---------------------------------------------------------------------
        $type$ oneToSevenHex = ($type$)1234567;
        test("%x", "null", (Object)null);
        test("%x", "12d687", oneToSevenHex);
        test("%010x", "000012d687", oneToSevenHex);
        test("%-10x", "12d687    ", oneToSevenHex);
        test("%#10x", "  0x12d687", oneToSevenHex);
        test("%#10X", "  0X12D687",oneToSevenHex);
        test("%X", "12D687", oneToSevenHex);

        test("%018x",    "00ffffffffffffff80", minByte);
        test("%-18x",      "ffffffffffffff80  ", minByte);
        test("%#20x",  "  0xffffffffffffff80", minByte);
        test("%0#20x", "0x00ffffffffffffff80", minByte);
        test("%#20X", "  0XFFFFFFFFFFFFFF80", minByte);
        test("%X",        "FFFFFFFFFFFFFF80", minByte);
#end[long]
        //---------------------------------------------------------------------
        // %x - errors
        //---------------------------------------------------------------------
        tryCatch("%,x", FormatFlagsConversionMismatchException.class);
        tryCatch("%0x", MissingFormatWidthException.class);
        tryCatch("%-x", MissingFormatWidthException.class);

#end[dec]

#if[BigInteger]
        //---------------------------------------------------------------------
        // BigInteger - errors
        //---------------------------------------------------------------------
        tryCatch("%f", IllegalFormatConversionException.class,
                 new BigInteger("1"));

        //---------------------------------------------------------------------
        // %d - BigInteger
        //---------------------------------------------------------------------
        test("%d", "null", (Object)null);
        test("%d", "1234567", new BigInteger("1234567", 10));
        test("%,d", "1,234,567", new BigInteger("1234567", 10));
        test(Locale.FRANCE, "%,d", "1\u00a0234\u00a0567", new BigInteger("1234567", 10));
        test("%,d", "-1,234,567", new BigInteger("-1234567", 10));
        test("%(d", "1234567", new BigInteger("1234567", 10));
        test("%(d", "(1234567)", new BigInteger("-1234567", 10));
        test("% d", " 1234567", new BigInteger("1234567", 10));
        test("% d", "-1234567", new BigInteger("-1234567", 10));
        test("%+d", "+1234567", new BigInteger("1234567", 10));
        test("%+d", "-1234567", new BigInteger("-1234567", 10));
        test("%010d", "0001234567", new BigInteger("1234567", 10));
        test("%010d", "-001234567", new BigInteger("-1234567", 10));
        test("%(10d", " (1234567)", new BigInteger("-1234567", 10));
        test("%+d", "+1234567", new BigInteger("1234567", 10));
        test("%+d", "-1234567", new BigInteger("-1234567", 10));
        test("%-10d", "1234567   ", new BigInteger("1234567", 10));
        test("%-10d", "-1234567  ", new BigInteger("-1234567", 10));

        //---------------------------------------------------------------------
        // %o - BigInteger
        //---------------------------------------------------------------------
        test("%o", "null", (Object)null);
        test("%o", "1234567", new BigInteger("1234567", 8));
        test("%(o", "1234567", new BigInteger("1234567", 8));
        test("%(o", "(1234567)", new BigInteger("-1234567", 8));
        test("% o", " 1234567", new BigInteger("1234567", 8));
        test("% o", "-1234567", new BigInteger("-1234567", 8));
        test("%+o", "+1234567", new BigInteger("1234567", 8));
        test("%+o", "-1234567", new BigInteger("-1234567", 8));
        test("%010o", "0001234567", new BigInteger("1234567", 8));
        test("%010o", "-001234567", new BigInteger("-1234567", 8));
        test("%(10o", " (1234567)", new BigInteger("-1234567", 8));
        test("%+o", "+1234567", new BigInteger("1234567", 8));
        test("%+o", "-1234567", new BigInteger("-1234567", 8));
        test("%-10o", "1234567   ", new BigInteger("1234567", 8));
        test("%-10o", "-1234567  ", new BigInteger("-1234567", 8));
        test("%#10o", "  01234567", new BigInteger("1234567", 8));
        test("%#10o", " -01234567", new BigInteger("-1234567", 8));

        //---------------------------------------------------------------------
        // %x - BigInteger
        //---------------------------------------------------------------------
        test("%x", "null", (Object)null);
        test("%x", "1234567", new BigInteger("1234567", 16));
        test("%(x", "1234567", new BigInteger("1234567", 16));
        test("%(x", "(1234567)", new BigInteger("-1234567", 16));
        test("% x", " 1234567", new BigInteger("1234567", 16));
        test("% x", "-1234567", new BigInteger("-1234567", 16));
        test("%+x", "+1234567", new BigInteger("1234567", 16));
        test("%+x", "-1234567", new BigInteger("-1234567", 16));
        test("%010x", "0001234567", new BigInteger("1234567", 16));
        test("%010x", "-001234567", new BigInteger("-1234567", 16));
        test("%(10x", " (1234567)", new BigInteger("-1234567", 16));
        test("%+x", "+1234567", new BigInteger("1234567", 16));
        test("%+x", "-1234567", new BigInteger("-1234567", 16));
        test("%-10x", "1234567   ", new BigInteger("1234567", 16));
        test("%-10x", "-1234567  ", new BigInteger("-1234567", 16));
        test("%#10x", " 0x1234567", new BigInteger("1234567", 16));
        test("%#10x", "-0x1234567", new BigInteger("-1234567", 16));
        test("%#10X", " 0X1234567", new BigInteger("1234567", 16));
        test("%#10X", "-0X1234567", new BigInteger("-1234567", 16));
        test("%X", "1234567A", new BigInteger("1234567a", 16));
        test("%X", "-1234567A", new BigInteger("-1234567a", 16));
#end[BigInteger]

#if[fp]
#if[BigDecimal]
        //---------------------------------------------------------------------
        // %s - BigDecimal
        //---------------------------------------------------------------------
        $type$ one = BigDecimal.ONE;
        $type$ ten = BigDecimal.TEN;
        $type$ pi  = new $type$(Math.PI);
        $type$ piToThe300 = pi.pow(300);

        test("%s", "3.141592653589793115997963468544185161590576171875", pi);
#end[BigDecimal]
#if[float]
        //---------------------------------------------------------------------
        // %s - float
        //---------------------------------------------------------------------
        $type$ one = 1.0f;
        $type$ ten = 10.0f;
        $type$ pi  = (float) Math.PI;

        test("%s", "3.1415927", pi);
#end[float]
#if[Float]
        //---------------------------------------------------------------------
        // %s - Float
        //---------------------------------------------------------------------
        $type$ one = new $type$(1.0f);
        $type$ ten = new $type$(10.0f);
        $type$ pi  = new $type$(Math.PI);

        test("%s", "3.1415927", pi);
#end[Float]
#if[double]
        //---------------------------------------------------------------------
        // %s - double
        //---------------------------------------------------------------------
        $type$ one = 1.0;
        $type$ ten = 10.0;
        $type$ pi  = Math.PI;

        test("%s", "3.141592653589793", pi);
#end[double]
#if[Double]
        //---------------------------------------------------------------------
        // %s - Double
        //---------------------------------------------------------------------
        $type$ one = new $type$(1.0);
        $type$ ten = new $type$(10.0);
        $type$ pi  = new $type$(Math.PI);

        test("%s", "3.141592653589793", pi);
#end[Double]

        //---------------------------------------------------------------------
        // flag/conversion errors
        //---------------------------------------------------------------------
        tryCatch("%d", IllegalFormatConversionException.class, one);
        tryCatch("%,.4e", FormatFlagsConversionMismatchException.class, one);

        //---------------------------------------------------------------------
        // %e
        //
        // Floating-point conversions applicable to float, double, and
        // BigDecimal.
        //---------------------------------------------------------------------
        test("%e", "null", (Object)null);

        //---------------------------------------------------------------------
        // %e - float and double
        //---------------------------------------------------------------------
        // double PI = 3.141 592 653 589 793 238 46;
        test("%e", "3.141593e+00", pi);
        test("%.0e", "1e+01", ten);
        test("%#.0e", "1.e+01", ten);
        test("%E", "3.141593E+00", pi);
        test("%10.3e", " 3.142e+00", pi);
        test("%10.3e", "-3.142e+00", negate(pi));
        test("%010.3e", "03.142e+00", pi);
        test("%010.3e", "-3.142e+00", negate(pi));
        test("%-12.3e", "3.142e+00   ", pi);
        test("%-12.3e", "-3.142e+00  ", negate(pi));
        test("%.3e", "3.142e+00", pi);
        test("%.3e", "-3.142e+00", negate(pi));
        test("%.3e", "3.142e+06", mult(pi, 1000000.0));
        test("%.3e", "-3.142e+06", mult(pi, -1000000.0));

        test(Locale.FRANCE, "%e", "3,141593e+00", pi);

        // double PI^300
        //    = 13962455701329742638131355433930076081862072808 ... e+149
#if[BigDecimal]
        //---------------------------------------------------------------------
        // %e - BigDecimal
        //---------------------------------------------------------------------
        test("%.3e", "1.396e+149", piToThe300);
        test("%.3e", "-1.396e+149", piToThe300.negate());
        test("%.3e", "1.000e-100", recip(ten.pow(100)));
        test("%.3e", "-1.000e-100", negate(recip(ten.pow(100))));

        test("%3.0e", "1e-06", new BigDecimal("0.000001"));
        test("%3.0e", "1e-05", new BigDecimal("0.00001"));
        test("%3.0e", "1e-04", new BigDecimal("0.0001"));
        test("%3.0e", "1e-03", new BigDecimal("0.001"));
        test("%3.0e", "1e-02", new BigDecimal("0.01"));
        test("%3.0e", "1e-01", new BigDecimal("0.1"));
        test("%3.0e", "9e-01", new BigDecimal("0.9"));
        test("%3.1e", "9.0e-01", new BigDecimal("0.9"));
        test("%3.0e", "1e+00", new BigDecimal("1.00"));
        test("%3.0e", "1e+01", new BigDecimal("10.00"));
        test("%3.0e", "1e+02", new BigDecimal("99.19"));
        test("%3.1e", "9.9e+01", new BigDecimal("99.19"));
        test("%3.0e", "1e+02", new BigDecimal("99.99"));
        test("%3.0e", "1e+02", new BigDecimal("100.00"));
        test("%#3.0e", "1.e+03",    new BigDecimal("1000.00"));
        test("%3.0e", "1e+04",     new BigDecimal("10000.00"));
        test("%3.0e", "1e+05",    new BigDecimal("100000.00"));
        test("%3.0e", "1e+06",   new BigDecimal("1000000.00"));
        test("%3.0e", "1e+07",  new BigDecimal("10000000.00"));
        test("%3.0e", "1e+08", new BigDecimal("100000000.00"));
#end[BigDecimal]

        test("%10.3e", " 1.000e+00", one);
        test("%+.3e", "+3.142e+00", pi);
        test("%+.3e", "-3.142e+00", negate(pi));
        test("% .3e", " 3.142e+00", pi);
        test("% .3e", "-3.142e+00", negate(pi));
        test("%#.0e", "3.e+00", create(3.0));
        test("%#.0e", "-3.e+00", create(-3.0));
        test("%.0e", "3e+00", create(3.0));
        test("%.0e", "-3e+00", create(-3.0));

        test("%(.4e", "3.1416e+06", mult(pi, 1000000.0));
        test("%(.4e", "(3.1416e+06)", mult(pi, -1000000.0));

        //---------------------------------------------------------------------
        // %e - boundary problems
        //---------------------------------------------------------------------
        test("%3.0e", "1e-06", 0.000001);
        test("%3.0e", "1e-05", 0.00001);
        test("%3.0e", "1e-04", 0.0001);
        test("%3.0e", "1e-03", 0.001);
        test("%3.0e", "1e-02", 0.01);
        test("%3.0e", "1e-01", 0.1);
        test("%3.0e", "9e-01", 0.9);
        test("%3.1e", "9.0e-01", 0.9);
        test("%3.0e", "1e+00", 1.00);
        test("%3.0e", "1e+01", 10.00);
        test("%3.0e", "1e+02", 99.19);
        test("%3.1e", "9.9e+01", 99.19);
        test("%3.0e", "1e+02", 99.99);
        test("%3.0e", "1e+02", 100.00);
        test("%#3.0e", "1.e+03",     1000.00);
        test("%3.0e", "1e+04",     10000.00);
        test("%3.0e", "1e+05",    100000.00);
        test("%3.0e", "1e+06",   1000000.00);
        test("%3.0e", "1e+07",  10000000.00);
        test("%3.0e", "1e+08", 100000000.00);

        //---------------------------------------------------------------------
        // %f
        //
        // Floating-point conversions applicable to float, double, and
        // BigDecimal.
        //---------------------------------------------------------------------
        test("%f", "null", (Object)null);
        test("%f", "3.141593", pi);
        test(Locale.FRANCE, "%f", "3,141593", pi);
        test("%10.3f", "     3.142", pi);
        test("%10.3f", "    -3.142", negate(pi));
        test("%010.3f", "000003.142", pi);
        test("%010.3f", "-00003.142", negate(pi));
        test("%-10.3f", "3.142     ", pi);
        test("%-10.3f", "-3.142    ", negate(pi));
        test("%.3f", "3.142", pi);
        test("%.3f", "-3.142", negate(pi));
        test("%+.3f", "+3.142", pi);
        test("%+.3f", "-3.142", negate(pi));
        test("% .3f", " 3.142", pi);
        test("% .3f", "-3.142", negate(pi));
        test("%#.0f", "3.", create(3.0));
        test("%#.0f", "-3.", create(-3.0));
        test("%.0f", "3", create(3.0));
        test("%.0f", "-3", create(-3.0));
        test("%.3f", "10.000", ten);
        test("%.3f", "1.000", one);
        test("%10.3f", "     1.000", one);

        //---------------------------------------------------------------------
        // %f - boundary problems
        //---------------------------------------------------------------------
        test("%3.0f", "  0", 0.000001);
        test("%3.0f", "  0", 0.00001);
        test("%3.0f", "  0", 0.0001);
        test("%3.0f", "  0", 0.001);
        test("%3.0f", "  0", 0.01);
        test("%3.0f", "  0", 0.1);
        test("%3.0f", "  1", 0.9);
        test("%3.1f", "0.9", 0.9);
        test("%3.0f", "  1", 1.00);
        test("%3.0f", " 10", 10.00);
        test("%3.0f", " 99", 99.19);
        test("%3.1f", "99.2", 99.19);
        test("%3.0f", "100", 99.99);
        test("%3.0f", "100", 100.00);
        test("%#3.0f", "1000.",     1000.00);
        test("%3.0f", "10000",     10000.00);
        test("%3.0f", "100000",    100000.00);
        test("%3.0f", "1000000",   1000000.00);
        test("%3.0f", "10000000",  10000000.00);
        test("%3.0f", "100000000", 100000000.00);
#if[BigDecimal]
        //---------------------------------------------------------------------
        // %f - BigDecimal
        //---------------------------------------------------------------------
        test("%4.0f", "  99", new BigDecimal("99.19"));
        test("%4.1f", "99.2", new BigDecimal("99.19"));

        BigDecimal val = new BigDecimal("99.95");
        test("%4.0f", " 100", val);
        test("%#4.0f", "100.", val);
        test("%4.1f", "100.0", val);
        test("%4.2f", "99.95", val);
        test("%4.3f", "99.950", val);

        val = new BigDecimal(".99");
        test("%4.1f", " 1.0", val);
        test("%4.2f", "0.99", val);
        test("%4.3f", "0.990", val);

        // #6476425
        val = new BigDecimal("0.00001");
        test("%.0f", "0", val);
        test("%.1f", "0.0", val);
        test("%.2f", "0.00", val);
        test("%.3f", "0.000", val);
        test("%.4f", "0.0000", val);
        test("%.5f", "0.00001", val);

        val = new BigDecimal("1.00001");
        test("%.0f", "1", val);
        test("%.1f", "1.0", val);
        test("%.2f", "1.00", val);
        test("%.3f", "1.000", val);
        test("%.4f", "1.0000", val);
        test("%.5f", "1.00001", val);

        val = new BigDecimal("1.23456");
        test("%.0f", "1", val);
        test("%.1f", "1.2", val);
        test("%.2f", "1.23", val);
        test("%.3f", "1.235", val);
        test("%.4f", "1.2346", val);
        test("%.5f", "1.23456", val);
        test("%.6f", "1.234560", val);

        val = new BigDecimal("9.99999");
        test("%.0f", "10", val);
        test("%.1f", "10.0", val);
        test("%.2f", "10.00", val);
        test("%.3f", "10.000", val);
        test("%.4f", "10.0000", val);
        test("%.5f", "9.99999", val);
        test("%.6f", "9.999990", val);


        val = new BigDecimal("1.99999");
        test("%.0f", "2", val);
        test("%.1f", "2.0", val);
        test("%.2f", "2.00", val);
        test("%.3f", "2.000", val);
        test("%.4f", "2.0000", val);
        test("%.5f", "1.99999", val);
        test("%.6f", "1.999990", val);

#end[BigDecimal]

#if[float]
        //---------------------------------------------------------------------
        // %f - float
        //---------------------------------------------------------------------
        // Float can not accurately store 1e6 * PI.
        test("%.3f", "3141.593", mult(pi, 1000.0));
        test("%.3f", "-3141.593", mult(pi, -1000.0));

        test("%,.2f", "3,141.59", mult(pi, 1000.0));
        test(Locale.FRANCE, "%,.2f", "3\u00a0141,59", mult(pi, 1000.0));
        test("%,.2f", "-3,141.59", mult(pi, -1000.0));
        test("%(.2f", "3141.59", mult(pi, 1000.0));
        test("%(.2f", "(3141.59)", mult(pi, -1000.0));
        test("%(,.2f", "3,141.59", mult(pi, 1000.0));
        test("%(,.2f", "(3,141.59)", mult(pi, -1000.0));

#else[float]
#if[!Float]
        //---------------------------------------------------------------------
        // %f - float, double, Double, BigDecimal
        //---------------------------------------------------------------------
        test("%.3f", "3141592.654", mult(pi, 1000000.0));
        test("%.3f", "-3141592.654", mult(pi, -1000000.0));
        test("%,.4f", "3,141,592.6536", mult(pi, 1000000.0));
        test(Locale.FRANCE, "%,.4f", "3\u00a0141\u00a0592,6536", mult(pi, 1000000.0));
        test("%,.4f", "-3,141,592.6536", mult(pi, -1000000.0));
        test("%(.4f", "3141592.6536", mult(pi, 1000000.0));
        test("%(.4f", "(3141592.6536)", mult(pi, -1000000.0));
        test("%(,.4f", "3,141,592.6536", mult(pi, 1000000.0));
        test("%(,.4f", "(3,141,592.6536)", mult(pi, -1000000.0));
#end[!Float]
#end[float]


        //---------------------------------------------------------------------
        // %g
        //
        // Floating-point conversions applicable to float, double, and
        // BigDecimal.
        //---------------------------------------------------------------------
        test("%g", "null", (Object)null);
        test("%g", "3.14159", pi);
        test(Locale.FRANCE, "%g", "3,14159", pi);
        test("%.0g", "1e+01", ten);
        test("%G", "3.14159", pi);
        test("%10.3g", "      3.14", pi);
        test("%10.3g", "     -3.14", negate(pi));
        test("%010.3g", "0000003.14", pi);
        test("%010.3g", "-000003.14", negate(pi));
        test("%-12.3g", "3.14        ", pi);
        test("%-12.3g", "-3.14       ", negate(pi));
        test("%.3g", "3.14", pi);
        test("%.3g", "-3.14", negate(pi));
        test("%.3g", "3.14e+08", mult(pi, 100000000.0));
        test("%.3g", "-3.14e+08", mult(pi, -100000000.0));

        test("%.3g", "1.00e-05", recip(create(100000.0)));
        test("%.3g", "-1.00e-05", recip(create(-100000.0)));
        test("%.0g", "-1e-05", recip(create(-100000.0)));
        test("%.0g", "1e+05", create(100000.0));
        test("%.3G", "1.00E-05", recip(create(100000.0)));
        test("%.3G", "-1.00E-05", recip(create(-100000.0)));

        test("%3.0g", "1e-06", 0.000001);
        test("%3.0g", "1e-05", 0.00001);
        test("%3.0g", "1e-05", 0.0000099);
        test("%3.1g", "1e-05", 0.0000099);
        test("%3.2g", "9.9e-06", 0.0000099);
        test("%3.0g", "0.0001", 0.0001);
        test("%3.0g", "9e-05",  0.00009);
        test("%3.0g", "0.0001", 0.000099);
        test("%3.1g", "0.0001", 0.000099);
        test("%3.2g", "9.9e-05", 0.000099);
        test("%3.0g", "0.001", 0.001);
        test("%3.0g", "0.001", 0.00099);
        test("%3.1g", "0.001", 0.00099);
        test("%3.2g", "0.00099", 0.00099);
        test("%3.3g", "0.00100", 0.001);
        test("%3.4g", "0.001000", 0.001);
        test("%3.0g", "0.01", 0.01);
        test("%3.0g", "0.1", 0.1);
        test("%3.0g", "0.9", 0.9);
        test("%3.1g", "0.9", 0.9);
        test("%3.0g", "  1", 1.00);
        test("%3.2g", " 10", 10.00);
        test("%3.0g", "1e+01", 10.00);
        test("%3.0g", "1e+02", 99.19);
        test("%3.1g", "1e+02", 99.19);
        test("%3.2g", " 99", 99.19);
        test("%3.0g", "1e+02", 99.9);
        test("%3.1g", "1e+02", 99.9);
        test("%3.2g", "1.0e+02", 99.9);
        test("%3.0g", "1e+02", 99.99);
        test("%3.0g", "1e+02", 100.00);
        test("%3.0g", "1e+03", 999.9);
        test("%3.1g", "1e+03", 999.9);
        test("%3.2g", "1.0e+03", 999.9);
        test("%3.3g", "1.00e+03", 999.9);
        test("%3.4g", "999.9", 999.9);
        test("%3.4g", "1000", 999.99);
        test("%3.0g", "1e+03", 1000.00);
        test("%3.0g", "1e+04",     10000.00);
        test("%3.0g", "1e+05",    100000.00);
        test("%3.0g", "1e+06",   1000000.00);
        test("%3.0g", "1e+07",  10000000.00);
        test("%3.9g", "100000000",  100000000.00);
        test("%3.10g", "100000000.0", 100000000.00);

        tryCatch("%#3.0g", FormatFlagsConversionMismatchException.class, 1000.00);

        // double PI^300
        //    = 13962455701329742638131355433930076081862072808 ... e+149
#if[BigDecimal]
        //---------------------------------------------------------------------
        // %g - BigDecimal
        //---------------------------------------------------------------------
        test("%.3g", "1.40e+149", piToThe300);
        test("%.3g", "-1.40e+149", piToThe300.negate());
        test(Locale.FRANCE, "%.3g", "-1,40e+149", piToThe300.negate());
        test("%.3g", "1.00e-100", recip(ten.pow(100)));
        test("%.3g", "-1.00e-100", negate(recip(ten.pow(100))));

        test("%3.0g", "1e-06", new BigDecimal("0.000001"));
        test("%3.0g", "1e-05", new BigDecimal("0.00001"));
        test("%3.0g", "0.0001", new BigDecimal("0.0001"));
        test("%3.0g", "0.001", new BigDecimal("0.001"));
        test("%3.3g", "0.00100", new BigDecimal("0.001"));
        test("%3.4g", "0.001000", new BigDecimal("0.001"));
        test("%3.0g", "0.01", new BigDecimal("0.01"));
        test("%3.0g", "0.1", new BigDecimal("0.1"));
        test("%3.0g", "0.9", new BigDecimal("0.9"));
        test("%3.1g", "0.9", new BigDecimal("0.9"));
        test("%3.0g", "  1", new BigDecimal("1.00"));
        test("%3.2g", " 10", new BigDecimal("10.00"));
        test("%3.0g", "1e+01", new BigDecimal("10.00"));
        test("%3.0g", "1e+02", new BigDecimal("99.19"));
        test("%3.1g", "1e+02", new BigDecimal("99.19"));
        test("%3.2g", " 99", new BigDecimal("99.19"));
        test("%3.0g", "1e+02", new BigDecimal("99.99"));
        test("%3.0g", "1e+02", new BigDecimal("100.00"));
        test("%3.0g", "1e+03", new BigDecimal("1000.00"));
        test("%3.0g", "1e+04",      new BigDecimal("10000.00"));
        test("%3.0g", "1e+05",      new BigDecimal("100000.00"));
        test("%3.0g", "1e+06",      new BigDecimal("1000000.00"));
        test("%3.0g", "1e+07",      new BigDecimal("10000000.00"));
        test("%3.9g", "100000000",  new BigDecimal("100000000.00"));
        test("%3.10g", "100000000.0", new BigDecimal("100000000.00"));
#end[BigDecimal]

        test("%.3g", "10.0", ten);
        test("%.3g", "1.00", one);
        test("%10.3g", "      1.00", one);
        test("%+10.3g", "     +3.14", pi);
        test("%+10.3g", "     -3.14", negate(pi));
        test("% .3g", " 3.14", pi);
        test("% .3g", "-3.14", negate(pi));
        test("%.0g", "3", create(3.0));
        test("%.0g", "-3", create(-3.0));

        test("%(.4g", "3.142e+08", mult(pi, 100000000.0));
        test("%(.4g", "(3.142e+08)", mult(pi, -100000000.0));

#if[float]
        // Float can not accurately store 1e6 * PI.
        test("%,.6g", "3,141.59", mult(pi, 1000.0));
        test("%(,.6g", "(3,141.59)", mult(pi, -1000.0));
#else[float]
#if[!Float]
        test("%,.11g", "3,141,592.6536", mult(pi, 1000000.0));
        test("%(,.11g", "(3,141,592.6536)", mult(pi, -1000000.0));
#end[!Float]
#end[float]

#if[double]
        //---------------------------------------------------------------------
        // %a
        //
        // Floating-point conversions applicable to float, double, and
        // BigDecimal.
        //---------------------------------------------------------------------
        test("%a", "null", (Object)null);
        test("%.11a", "0x0.00000000000p0", 0.0);
        test(Locale.FRANCE, "%.11a", "0x0.00000000000p0", 0.0); // no localization
        test("%.1a", "0x0.0p0", 0.0);
        test("%.11a", "-0x0.00000000000p0", -0.0);
        test("%.1a", "-0x0.0p0", -0.0);
        test("%.11a", "0x1.00000000000p0", 1.0);
        test("%.1a", "0x1.0p0", 1.0);
        test("%.11a", "-0x1.00000000000p0", -1.0);
        test("%.1a", "-0x1.0p0", -1.0);
        test("%.11a", "0x1.80000000000p1", 3.0);
        test("%.1a", "0x1.8p1", 3.0);
        test("%.11a", "0x1.00000000000p-1022", DoubleConsts.MIN_NORMAL);
        test("%.1a", "0x1.0p-1022", DoubleConsts.MIN_NORMAL);
        test("%.11a", "0x1.00000000000p-1022",
             FpUtils.nextDown(DoubleConsts.MIN_NORMAL));
        test("%.1a", "0x1.0p-1022",
             FpUtils.nextDown(DoubleConsts.MIN_NORMAL));
        test("%.11a", "0x1.ffffffffffep-1023",
             Double.parseDouble("0x0.fffffffffffp-1022"));
        test("%.1a", "0x1.0p-1022",
             Double.parseDouble("0x0.fffffffffffp-1022"));
        test("%.30a", "0x0.000000000000100000000000000000p-1022", Double.MIN_VALUE);
        test("%.13a", "0x0.0000000000001p-1022", Double.MIN_VALUE);
        test("%.11a", "0x1.00000000000p-1074", Double.MIN_VALUE);
        test("%.1a", "0x1.0p-1074", Double.MIN_VALUE);

        test("%.11a", "0x1.08000000000p-1069",
             Double.MIN_VALUE + Double.MIN_VALUE*32);
        test("%.1a", "0x1.0p-1069",
             Double.MIN_VALUE + Double.MIN_VALUE*32);
        test("%.30a", "0x1.fffffffffffff00000000000000000p1023", Double.MAX_VALUE);
        test("%.13a", "0x1.fffffffffffffp1023", Double.MAX_VALUE);
        test("%.11a", "0x1.00000000000p1024", Double.MAX_VALUE);
        test("%.1a", "0x1.0p1024", Double.MAX_VALUE);
        test("%.11a", "0x1.18000000000p0", Double.parseDouble("0x1.18p0"));
        test("%.1a", "0x1.2p0", Double.parseDouble("0x1.18p0"));

        test("%.11a", "0x1.18000000000p0",
             Double.parseDouble("0x1.180000000001p0"));
        test("%.1a", "0x1.2p0",
             Double.parseDouble("0x1.180000000001p0"));
        test("%.11a", "0x1.28000000000p0", Double.parseDouble("0x1.28p0"));
        test("%.1a", "0x1.2p0", Double.parseDouble("0x1.28p0"));

        test("%.11a", "0x1.28000000000p0",
             Double.parseDouble("0x1.280000000001p0"));
        test("%.1a", "0x1.3p0", Double.parseDouble("0x1.280000000001p0"));
#end[double]

        //---------------------------------------------------------------------
        // %f, %e, %g, %a - Boundaries
        //---------------------------------------------------------------------
#if[float]
        //---------------------------------------------------------------------
        // %f, %e, %g, %a - NaN
        //---------------------------------------------------------------------
        test("%f", "NaN", Float.NaN);
        // s
        test("%+f", "NaN", Float.NaN);
//      test("%F", "NAN", Float.NaN);
        test("%e", "NaN", Float.NaN);
        test("%+e", "NaN", Float.NaN);
        test("%E", "NAN", Float.NaN);
        test("%g", "NaN", Float.NaN);
        test("%+g", "NaN", Float.NaN);
        test("%G", "NAN", Float.NaN);
        test("%a", "NaN", Float.NaN);
        test("%+a", "NaN", Float.NaN);
        test("%A", "NAN", Float.NaN);

        //---------------------------------------------------------------------
        // %f, %e, %g, %a - +0.0
        //---------------------------------------------------------------------
        test("%f", "0.000000", +0.0);
        test("%+f", "+0.000000", +0.0);
        test("% f", " 0.000000", +0.0);
//      test("%F", "0.000000", +0.0);
        test("%e", "0.000000e+00", 0e0);
        test("%e", "0.000000e+00", +0.0);
        test("%+e", "+0.000000e+00", +0.0);
        test("% e", " 0.000000e+00", +0.0);
        test("%E", "0.000000E+00", 0e0);
        test("%E", "0.000000E+00", +0.0);
        test("%+E", "+0.000000E+00", +0.0);
        test("% E", " 0.000000E+00", +0.0);
        test("%g", "0.00000", +0.0);
        test("%+g", "+0.00000", +0.0);
        test("% g", " 0.00000", +0.0);
        test("%G", "0.00000", +0.0);
        test("% G", " 0.00000", +0.0);
        test("%a", "0x0.0p0", +0.0);
        test("%+a", "+0x0.0p0", +0.0);
        test("% a", " 0x0.0p0", +0.0);
        test("%A", "0X0.0P0", +0.0);
        test("% A", " 0X0.0P0", +0.0);

        //---------------------------------------------------------------------
        // %f, %e, %g, %a - -0.0
        //---------------------------------------------------------------------
        test("%f", "-0.000000", -0.0);
        test("%+f", "-0.000000", -0.0);
//      test("%F", "-0.000000", -0.0);
        test("%e", "-0.000000e+00", -0.0);
        test("%+e", "-0.000000e+00", -0.0);
        test("%E", "-0.000000E+00", -0.0);
        test("%+E", "-0.000000E+00", -0.0);
        test("%g", "-0.00000", -0.0);
        test("%+g", "-0.00000", -0.0);
        test("%G", "-0.00000", -0.0);
        test("%a", "-0x0.0p0", -0.0);
        test("%+a", "-0x0.0p0", -0.0);
        test("%+A", "-0X0.0P0", -0.0);

        //---------------------------------------------------------------------
        // %f, %e, %g, %a - +Infinity
        //---------------------------------------------------------------------
        test("%f", "Infinity", Float.POSITIVE_INFINITY);
        test("%+f", "+Infinity", Float.POSITIVE_INFINITY);
        test("% f", " Infinity", Float.POSITIVE_INFINITY);
//      test("%F", "INFINITY", Float.POSITIVE_INFINITY);
        test("%e", "Infinity", Float.POSITIVE_INFINITY);
        test("%+e", "+Infinity", Float.POSITIVE_INFINITY);
        test("% e", " Infinity", Float.POSITIVE_INFINITY);
        test("%E", "INFINITY", Float.POSITIVE_INFINITY);
        test("%+E", "+INFINITY", Float.POSITIVE_INFINITY);
        test("% E", " INFINITY", Float.POSITIVE_INFINITY);
        test("%g", "Infinity", Float.POSITIVE_INFINITY);
        test("%+g", "+Infinity", Float.POSITIVE_INFINITY);
        test("%G", "INFINITY", Float.POSITIVE_INFINITY);
        test("% G", " INFINITY", Float.POSITIVE_INFINITY);
        test("%+G", "+INFINITY", Float.POSITIVE_INFINITY);
        test("%a", "Infinity", Float.POSITIVE_INFINITY);
        test("%+a", "+Infinity", Float.POSITIVE_INFINITY);
        test("% a", " Infinity", Float.POSITIVE_INFINITY);
        test("%A", "INFINITY", Float.POSITIVE_INFINITY);
        test("%+A", "+INFINITY", Float.POSITIVE_INFINITY);
        test("% A", " INFINITY", Float.POSITIVE_INFINITY);

        //---------------------------------------------------------------------
        // %f, %e, %g, %a - -Infinity
        //---------------------------------------------------------------------
        test("%f", "-Infinity", Float.NEGATIVE_INFINITY);
        test("%+f", "-Infinity", Float.NEGATIVE_INFINITY);
        test("%(f", "(Infinity)", Float.NEGATIVE_INFINITY);
//      test("%F", "-INFINITY", Float.NEGATIVE_INFINITY);
        test("%e", "-Infinity", Float.NEGATIVE_INFINITY);
        test("%+e", "-Infinity", Float.NEGATIVE_INFINITY);
        test("%E", "-INFINITY", Float.NEGATIVE_INFINITY);
        test("%+E", "-INFINITY", Float.NEGATIVE_INFINITY);
        test("%g", "-Infinity", Float.NEGATIVE_INFINITY);
        test("%+g", "-Infinity", Float.NEGATIVE_INFINITY);
        test("%G", "-INFINITY", Float.NEGATIVE_INFINITY);
        test("%+G", "-INFINITY", Float.NEGATIVE_INFINITY);
        test("%a", "-Infinity", Float.NEGATIVE_INFINITY);
        test("%+a", "-Infinity", Float.NEGATIVE_INFINITY);
        test("%A", "-INFINITY", Float.NEGATIVE_INFINITY);
        test("%+A", "-INFINITY", Float.NEGATIVE_INFINITY);

        //---------------------------------------------------------------------
        // %f, %e, %g, %a - Float.MIN_VALUE
        //---------------------------------------------------------------------
        test("%f", "0.000000", Float.MIN_VALUE);
        test("%,f", "0.000000", Float.MIN_VALUE);
        test("%(f", "(0.000000)", -Float.MIN_VALUE);
        test("%30.0f",  "                             0", Float.MIN_VALUE);
        test("%30.5f",  "                       0.00000", Float.MIN_VALUE);
        test("%30.13f", "               0.0000000000000", Float.MIN_VALUE);
        test("%30.20f", "        0.00000000000000000000", Float.MIN_VALUE);
        test("%e", "1.401298e-45", Float.MIN_VALUE);
        test("%E", "1.401298E-45", Float.MIN_VALUE);
        test("%(.1e", "1.4e-45", Float.MIN_VALUE);
        test("%(E", "(1.401298E-45)", -Float.MIN_VALUE);
        test("%30.5e",  "                   1.40130e-45", Float.MIN_VALUE);
        test("%30.13e", "           1.4012984643248e-45", Float.MIN_VALUE);
        test("%30.20e", "    1.40129846432481700000e-45", Float.MIN_VALUE);
        test("%g", "1.40130e-45", Float.MIN_VALUE);
        test("%G", "1.40130E-45", Float.MIN_VALUE);
        test("%(g", "1.40130e-45", Float.MIN_VALUE);
        test("%,g", "1.40130e-45", Float.MIN_VALUE);
        test("%(G", "(1.40130E-45)", -Float.MIN_VALUE);
        test("%30.5g",  "                    1.4013e-45", Float.MIN_VALUE);
        test("%30.13g", "            1.401298464325e-45", Float.MIN_VALUE);
        test("%30.20g", "     1.4012984643248170000e-45", Float.MIN_VALUE);
        test("%a", "0x1.0p-149", Float.MIN_VALUE);
        test("%A", "0X1.0P-149", Float.MIN_VALUE);
        test("%20a", "          0x1.0p-149", Float.MIN_VALUE);

        //---------------------------------------------------------------------
        // %f, %e, %g, %a - Float.MAX_VALUE
        //---------------------------------------------------------------------
        test("%f", "340282346638528860000000000000000000000.000000", Float.MAX_VALUE);
        test("%,f","340,282,346,638,528,860,000,000,000,000,000,000,000.000000",
             Float.MAX_VALUE);
        test("%(f", "(340282346638528860000000000000000000000.000000)", -Float.MAX_VALUE);
        test("%60.5f",  "               340282346638528860000000000000000000000.00000",
             Float.MAX_VALUE);
        test("%60.13f", "       340282346638528860000000000000000000000.0000000000000",
             Float.MAX_VALUE);
        test("%61.20f", " 340282346638528860000000000000000000000.00000000000000000000",
             Float.MAX_VALUE);
        test("%e", "3.402823e+38", Float.MAX_VALUE);
        test("%E", "3.402823E+38", Float.MAX_VALUE);
        test("%(e", "3.402823e+38", Float.MAX_VALUE);
        test("%(e", "(3.402823e+38)", -Float.MAX_VALUE);
        test("%30.5e",  "                   3.40282e+38", Float.MAX_VALUE);
        test("%30.13e", "           3.4028234663853e+38", Float.MAX_VALUE);
        test("%30.20e", "    3.40282346638528860000e+38", Float.MAX_VALUE);
        test("%g", "3.40282e+38", Float.MAX_VALUE);
        test("%G", "3.40282E+38", Float.MAX_VALUE);
        test("%,g", "3.40282e+38", Float.MAX_VALUE);
        test("%(g", "(3.40282e+38)", -Float.MAX_VALUE);
        test("%30.5g",  "                    3.4028e+38", Float.MAX_VALUE);
        test("%30.13g", "            3.402823466385e+38", Float.MAX_VALUE);
        test("%30.20G", "     3.4028234663852886000E+38", Float.MAX_VALUE);
        test("%a", "0x1.fffffep127", Float.MAX_VALUE);
        test("%A", "0X1.FFFFFEP127", Float.MAX_VALUE);
        test("%20a","      0x1.fffffep127", Float.MAX_VALUE);

#end[float]

#if[double]
        //---------------------------------------------------------------------
        // %f, %e, %g, %a - Double.MIN_VALUE
        //---------------------------------------------------------------------
        test("%f", "0.000000", Double.MIN_VALUE);
        test("%,f", "0.000000", Double.MIN_VALUE);
        test("%(f", "(0.000000)", -Double.MIN_VALUE);
        test("%30.0f",  "                             0", Double.MIN_VALUE);
        test("%30.5f",  "                       0.00000", Double.MIN_VALUE);
        test("%30.13f", "               0.0000000000000", Double.MIN_VALUE);
        test("%30.20f", "        0.00000000000000000000", Double.MIN_VALUE);
        test("%30.350f","0.00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000490000000000000000000000000",
             Double.MIN_VALUE);
        test("%e", "4.900000e-324", Double.MIN_VALUE);
        test("%E", "4.900000E-324", Double.MIN_VALUE);
        test("%(.1e", "4.9e-324", Double.MIN_VALUE);
        test("%(E", "(4.900000E-324)", -Double.MIN_VALUE);
        test("%30.5e",  "                  4.90000e-324", Double.MIN_VALUE);
        test("%30.13e", "          4.9000000000000e-324", Double.MIN_VALUE);
        test("%30.20e", "   4.90000000000000000000e-324", Double.MIN_VALUE);
        test("%g", "4.90000e-324", Double.MIN_VALUE);
        test("%G", "4.90000E-324", Double.MIN_VALUE);
        test("%(g", "4.90000e-324", Double.MIN_VALUE);
        test("%,g", "4.90000e-324", Double.MIN_VALUE);
        test("%30.5g",  "                   4.9000e-324", Double.MIN_VALUE);
        test("%30.13g", "           4.900000000000e-324", Double.MIN_VALUE);
        test("%30.20g", "    4.9000000000000000000e-324", Double.MIN_VALUE);
        test("%a", "0x0.0000000000001p-1022", Double.MIN_VALUE);
        test("%A", "0X0.0000000000001P-1022", Double.MIN_VALUE);
        test("%30a",    "       0x0.0000000000001p-1022", Double.MIN_VALUE);

        //---------------------------------------------------------------------
        // %f, %e, %g, %a - Double.MAX_VALUE
        //---------------------------------------------------------------------
        test("%f", "179769313486231570000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000.000000",
             Double.MAX_VALUE);
        test("%,f", "179,769,313,486,231,570,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000.000000",
             Double.MAX_VALUE);
        test("%,(f", "(179,769,313,486,231,570,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000.000000)",
             -Double.MAX_VALUE);
        test("%,30.5f", "179,769,313,486,231,570,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000.00000",
             Double.MAX_VALUE);
        test("%30.13f", "179769313486231570000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000.0000000000000",
             Double.MAX_VALUE);
        test("%30.20f", "179769313486231570000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000.00000000000000000000",
             Double.MAX_VALUE);
        test("%e", "1.797693e+308", Double.MAX_VALUE);
        test("%E", "1.797693E+308", Double.MAX_VALUE);
        test("%(e", "1.797693e+308", Double.MAX_VALUE);
        test("%(e", "(1.797693e+308)", -Double.MAX_VALUE);
        test("%30.5e",  "                  1.79769e+308", Double.MAX_VALUE);
        test("%30.13e", "          1.7976931348623e+308", Double.MAX_VALUE);
        test("%30.20e", "   1.79769313486231570000e+308", Double.MAX_VALUE);
        test("%g", "1.79769e+308", Double.MAX_VALUE);
        test("%G", "1.79769E+308", Double.MAX_VALUE);
        test("%,g", "1.79769e+308", Double.MAX_VALUE);
        test("%(g", "(1.79769e+308)", -Double.MAX_VALUE);
        test("%30.5g",  "                   1.7977e+308", Double.MAX_VALUE);
        test("%30.13g", "           1.797693134862e+308", Double.MAX_VALUE);
        test("%30.20g", "    1.7976931348623157000e+308", Double.MAX_VALUE);
        test("%a", "0x1.fffffffffffffp1023", Double.MAX_VALUE);
        test("%A", "0X1.FFFFFFFFFFFFFP1023", Double.MAX_VALUE);
        test("%30a",    "        0x1.fffffffffffffp1023", Double.MAX_VALUE);
#end[double]

#end[fp]

        //---------------------------------------------------------------------
        // %t
        //
        // Date/Time conversions applicable to Calendar, Date, and long.
        //---------------------------------------------------------------------
        test("%tA", "null", (Object)null);
        test("%TA", "NULL", (Object)null);

        //---------------------------------------------------------------------
        // %t - errors
        //---------------------------------------------------------------------
        tryCatch("%t", UnknownFormatConversionException.class);
        tryCatch("%T", UnknownFormatConversionException.class);
        tryCatch("%tP", UnknownFormatConversionException.class);
        tryCatch("%TP", UnknownFormatConversionException.class);
        tryCatch("%.5tB", IllegalFormatPrecisionException.class);
        tryCatch("%#tB", FormatFlagsConversionMismatchException.class);
        tryCatch("%-tB", MissingFormatWidthException.class);

#if[datetime]
        //---------------------------------------------------------------------
        // %t - create test Calendar
        //---------------------------------------------------------------------

        // Get the supported ids for GMT-08:00 (Pacific Standard Time)
        String[] ids = TimeZone.getAvailableIDs(-8 * 60 * 60 * 1000);
        // Create a Pacific Standard Time time zone
        SimpleTimeZone tz = new SimpleTimeZone(-8 * 60 * 60 * 1000, ids[0]);
        // public GregorianCalendar(TimeZone zone, Locale aLocale);
        Calendar c0 = new GregorianCalendar(tz, Locale.US);
        // public final void set(int year, int month, int date,
        //     int hourOfDay, int minute, int second);
        c0.set(1995, MAY, 23, 19, 48, 34);
        c0.set(Calendar.MILLISECOND, 584);

        //---------------------------------------------------------------------
        // %t - Minutes, {nano,milli}*seconds
        //
        // testDateTime() verifies the expected output for all applicable types
        // (Calendar, Date, and long).  It also verifies output for "%t" and
        // "%T". Thus it is sufficient to invoke that method once per
        // conversion/expected output.
        //---------------------------------------------------------------------
        testDateTime("%tM", "48", c0);
        testDateTime("%tN", "584000000", c0);
        testDateTime("%tL", "584", c0);
//      testDateTime("%tQ", "801283714584", c0);

        testDateTime("%ts", String.valueOf(c0.getTimeInMillis() / 1000), c0);
        testDateTime("%tS", "34", c0);
        testDateTime("%tT", "19:48:34", c0);

        //---------------------------------------------------------------------
        // %t - Hours, morning/afternoon markers
        //
        // testHours() iterates through all twenty-four hours to verify
        // numeric return value and morning/afternoon markers.
        //---------------------------------------------------------------------
        testHours();

        //---------------------------------------------------------------------
        // %t - Portions of date [ day, month, dates, weeks ]
        //---------------------------------------------------------------------
        testDateTime("%ta", "Tue", c0);
        testDateTime("%tA", "Tuesday", c0);
        testDateTime("%tb", "May", c0);
        testDateTime("%tB", "May", c0);
        testDateTime("%tC", "19", c0);
        testDateTime("%td", "23", c0);
        testDateTime("%te", "23", c0);
        testDateTime("%th", "May", c0);
        testDateTime("%tj", "143", c0);
        testDateTime("%tm", "05", c0);
        testDateTime("%ty", "95", c0);
        testDateTime("%tY", "1995", c0);

        //---------------------------------------------------------------------
        // %t - TimeZone
        //---------------------------------------------------------------------
        testDateTime("%tz", "-0800", c0);
        testDateTime("%tZ", "PST", c0);

        //---------------------------------------------------------------------
        // %tz should always adjust for DST
        //---------------------------------------------------------------------
        TimeZone dtz = TimeZone.getDefault();

        // Artificial TimeZone based on PST with 3:15 DST always in effect
        TimeZone atz = new SimpleTimeZone(-8 * 60 * 60 * 1000, "AlwaysDST",
            JANUARY, 1, 0, 0, STANDARD_TIME,
            // 24hrs - 1m = 60 * 60 * 1000 * 24 - 1
            DECEMBER, 31, 0, 60 * 60 * 1000 * 24 - 1, STANDARD_TIME,
            (int)(60 * 60 * 1000 * 3.25));
        TimeZone.setDefault(atz);
        testDateTime("%tz", "-0445", Calendar.getInstance(atz));

        // Restore the TimeZone and verify
        TimeZone.setDefault(dtz);
        if (atz.hasSameRules(TimeZone.getDefault()))
            throw new RuntimeException("Default TimeZone not restored");

        //---------------------------------------------------------------------
        // %t - Composites
        //---------------------------------------------------------------------
        testDateTime("%tr", "07:48:34 PM", c0);
        testDateTime("%tR", "19:48", c0);
        testDateTime("%tc", "Tue May 23 19:48:34 PST 1995", c0);
        testDateTime("%tD", "05/23/95", c0);
        testDateTime("%tF", "1995-05-23", c0);
        testDateTime("%-12tF", "1995-05-23  ", c0);
        testDateTime("%12tF", "  1995-05-23", c0);
#end[datetime]

        //---------------------------------------------------------------------
        // %n
        //---------------------------------------------------------------------
        test("%n", System.getProperty("line.separator"), (Object)null);
        test("%n", System.getProperty("line.separator"), "");

        tryCatch("%,n", IllegalFormatFlagsException.class);
        tryCatch("%.n", UnknownFormatConversionException.class);
        tryCatch("%5.n", UnknownFormatConversionException.class);
        tryCatch("%5n", IllegalFormatWidthException.class);
        tryCatch("%.7n", IllegalFormatPrecisionException.class);
        tryCatch("%<n", IllegalFormatFlagsException.class);

        //---------------------------------------------------------------------
        // %%
        //---------------------------------------------------------------------
        test("%%", "%", (Object)null);
        test("%%", "%", "");
        tryCatch("%%%", UnknownFormatConversionException.class);
        // perhaps an IllegalFormatArgumentIndexException should be defined?
        tryCatch("%<%", IllegalFormatFlagsException.class);
    }
}
