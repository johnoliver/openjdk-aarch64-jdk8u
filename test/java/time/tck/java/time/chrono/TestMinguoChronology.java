/*
 * Copyright (c) 2012, 2013, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
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
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

/*
 * Copyright (c) 2008-2012, Stephen Colebourne & Michael Nascimento Santos
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither the name of JSR-310 nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package tck.java.time.chrono;

import static org.testng.Assert.assertEquals;
import static org.testng.Assert.assertFalse;
import static org.testng.Assert.assertTrue;

import java.time.DateTimeException;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.LocalTime;
import java.time.Month;
import java.time.ZoneOffset;
import java.time.chrono.MinguoChronology;
import java.time.chrono.MinguoDate;
import java.time.temporal.Adjusters;
import java.time.temporal.ChronoUnit;
import java.time.chrono.ChronoZonedDateTime;
import java.time.chrono.Chronology;
import java.time.chrono.ChronoLocalDate;
import java.time.chrono.ChronoLocalDateTime;
import java.time.chrono.IsoChronology;
import java.time.chrono.Era;
import java.time.Year;

import org.testng.Assert;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

/**
 * Test.
 */
@Test
public class TestMinguoChronology {

    private static final int YDIFF = 1911;
    //-----------------------------------------------------------------------
    // Chronology.ofName("Minguo")  Lookup by name
    //-----------------------------------------------------------------------
    @Test(groups={"tck"})
    public void test_chrono_byName() {
        Chronology c = MinguoChronology.INSTANCE;
        Chronology test = Chronology.of("Minguo");
        Assert.assertNotNull(test, "The Minguo calendar could not be found byName");
        Assert.assertEquals(test.getId(), "Minguo", "ID mismatch");
        Assert.assertEquals(test.getCalendarType(), "roc", "Type mismatch");
        Assert.assertEquals(test, c);
    }

    //-----------------------------------------------------------------------
    // creation, toLocalDate()
    //-----------------------------------------------------------------------
    @DataProvider(name="samples")
    Object[][] data_samples() {
        return new Object[][] {
            {MinguoChronology.INSTANCE.date(1, 1, 1), LocalDate.of(1 + YDIFF, 1, 1)},
            {MinguoChronology.INSTANCE.date(1, 1, 2), LocalDate.of(1 + YDIFF, 1, 2)},
            {MinguoChronology.INSTANCE.date(1, 1, 3), LocalDate.of(1 + YDIFF, 1, 3)},

            {MinguoChronology.INSTANCE.date(2, 1, 1), LocalDate.of(2 + YDIFF, 1, 1)},
            {MinguoChronology.INSTANCE.date(3, 1, 1), LocalDate.of(3 + YDIFF, 1, 1)},
            {MinguoChronology.INSTANCE.date(3, 12, 6), LocalDate.of(3 + YDIFF, 12, 6)},
            {MinguoChronology.INSTANCE.date(4, 1, 1), LocalDate.of(4 + YDIFF, 1, 1)},
            {MinguoChronology.INSTANCE.date(4, 7, 3), LocalDate.of(4 + YDIFF, 7, 3)},
            {MinguoChronology.INSTANCE.date(4, 7, 4), LocalDate.of(4 + YDIFF, 7, 4)},
            {MinguoChronology.INSTANCE.date(5, 1, 1), LocalDate.of(5 + YDIFF, 1, 1)},
            {MinguoChronology.INSTANCE.date(100, 3, 3), LocalDate.of(100 + YDIFF, 3, 3)},
            {MinguoChronology.INSTANCE.date(101, 10, 28), LocalDate.of(101 + YDIFF, 10, 28)},
            {MinguoChronology.INSTANCE.date(101, 10, 29), LocalDate.of(101 + YDIFF, 10, 29)},

            {MinguoChronology.INSTANCE.dateYearDay(1916 - YDIFF, 60), LocalDate.of(1916, 2, 29)},
            {MinguoChronology.INSTANCE.dateYearDay(1908 - YDIFF, 60), LocalDate.of(1908, 2, 29)},
            {MinguoChronology.INSTANCE.dateYearDay(2000 - YDIFF, 60), LocalDate.of(2000, 2, 29)},
            {MinguoChronology.INSTANCE.dateYearDay(2400 - YDIFF, 60), LocalDate.of(2400, 2, 29)},
        };
    }

    @Test(dataProvider="samples", groups={"tck"})
    public void test_toLocalDate(MinguoDate minguo, LocalDate iso) {
        assertEquals(LocalDate.from(minguo), iso);
    }

    @Test(dataProvider="samples", groups={"tck"})
    public void test_fromCalendrical(MinguoDate minguo, LocalDate iso) {
        assertEquals(MinguoChronology.INSTANCE.date(iso), minguo);
    }

    @SuppressWarnings("unused")
    @Test(dataProvider="samples", groups={"implementation"})
    public void test_MinguoDate(MinguoDate minguoDate, LocalDate iso) {
        MinguoDate hd = minguoDate;
        ChronoLocalDateTime<MinguoDate> hdt = hd.atTime(LocalTime.NOON);
        ZoneOffset zo = ZoneOffset.ofHours(1);
        ChronoZonedDateTime<MinguoDate> hzdt = hdt.atZone(zo);
        hdt = hdt.plus(1, ChronoUnit.YEARS);
        hdt = hdt.plus(1, ChronoUnit.MONTHS);
        hdt = hdt.plus(1, ChronoUnit.DAYS);
        hdt = hdt.plus(1, ChronoUnit.HOURS);
        hdt = hdt.plus(1, ChronoUnit.MINUTES);
        hdt = hdt.plus(1, ChronoUnit.SECONDS);
        hdt = hdt.plus(1, ChronoUnit.NANOS);
        ChronoLocalDateTime<MinguoDate> a2 = hzdt.toLocalDateTime();
        MinguoDate a3 = a2.toLocalDate();
        MinguoDate a5 = hzdt.toLocalDate();
        //System.out.printf(" d: %s, dt: %s; odt: %s; zodt: %s; a4: %s%n", date, hdt, hodt, hzdt, a5);
    }

    @Test()
    public void test_MinguoChrono() {
        MinguoDate h1 = (MinguoDate)MinguoChronology.ERA_ROC.date(1, 2, 3);
        MinguoDate h2 = h1;
        ChronoLocalDateTime<MinguoDate> h3 = h2.atTime(LocalTime.NOON);
        @SuppressWarnings("unused")
        ChronoZonedDateTime<MinguoDate> h4 = h3.atZone(ZoneOffset.UTC);
    }

    @DataProvider(name="badDates")
    Object[][] data_badDates() {
        return new Object[][] {
            {1912, 0, 0},

            {1912, -1, 1},
            {1912, 0, 1},
            {1912, 14, 1},
            {1912, 15, 1},

            {1912, 1, -1},
            {1912, 1, 0},
            {1912, 1, 32},
            {1912, 2, 29},
            {1912, 2, 30},

            {1912, 12, -1},
            {1912, 12, 0},
            {1912, 12, 32},

            {1907 - YDIFF, 2, 29},
            {100 - YDIFF, 2, 29},
            {2100 - YDIFF, 2, 29},
            {2101 - YDIFF, 2, 29},
            };
    }

    @Test(dataProvider="badDates", groups={"tck"}, expectedExceptions=DateTimeException.class)
    public void test_badDates(int year, int month, int dom) {
        MinguoChronology.INSTANCE.date(year, month, dom);
    }

    //-----------------------------------------------------------------------
    // prolepticYear() and is LeapYear()
    //-----------------------------------------------------------------------
    @DataProvider(name="prolepticYear")
    Object[][] data_prolepticYear() {
        return new Object[][] {
            {1, MinguoChronology.ERA_ROC, 1912 - YDIFF, 1912 - YDIFF, true},
            {1, MinguoChronology.ERA_ROC, 1916 - YDIFF, 1916 - YDIFF, true},
            {1, MinguoChronology.ERA_ROC, 1914 - YDIFF, 1914 - YDIFF, false},
            {1, MinguoChronology.ERA_ROC, 2000 - YDIFF, 2000 - YDIFF, true},
            {1, MinguoChronology.ERA_ROC, 2100 - YDIFF, 2100 - YDIFF, false},
            {1, MinguoChronology.ERA_ROC, 0, 0, false},
            {1, MinguoChronology.ERA_ROC, 1908 - YDIFF, 1908 - YDIFF, true},
            {1, MinguoChronology.ERA_ROC, 1900 - YDIFF, 1900 - YDIFF, false},
            {1, MinguoChronology.ERA_ROC, 1600 - YDIFF, 1600 - YDIFF, true},

            {0, MinguoChronology.ERA_BEFORE_ROC, YDIFF - 1911, 1912 - YDIFF, true},
            {0, MinguoChronology.ERA_BEFORE_ROC, YDIFF - 1915, 1916 - YDIFF, true},
            {0, MinguoChronology.ERA_BEFORE_ROC, YDIFF - 1913, 1914 - YDIFF, false},
            {0, MinguoChronology.ERA_BEFORE_ROC, YDIFF - 1999, 2000 - YDIFF, true},
            {0, MinguoChronology.ERA_BEFORE_ROC, YDIFF - 2099, 2100 - YDIFF, false},
            {0, MinguoChronology.ERA_BEFORE_ROC, 1, 0, false},
            {0, MinguoChronology.ERA_BEFORE_ROC, YDIFF - 1907, 1908 - YDIFF, true},
            {0, MinguoChronology.ERA_BEFORE_ROC, YDIFF - 1899, 1900 - YDIFF, false},
            {0, MinguoChronology.ERA_BEFORE_ROC, YDIFF - 1599, 1600 - YDIFF, true},

        };
    }

    @Test(dataProvider="prolepticYear", groups={"tck"})
    public void test_prolepticYear(int eraValue, Era  era, int yearOfEra, int expectedProlepticYear, boolean isLeapYear) {
        Era eraObj = MinguoChronology.INSTANCE.eraOf(eraValue) ;
        assertTrue(MinguoChronology.INSTANCE.eras().contains(eraObj));
        assertEquals(eraObj, era);
        assertEquals(MinguoChronology.INSTANCE.prolepticYear(era, yearOfEra), expectedProlepticYear);
        assertEquals(MinguoChronology.INSTANCE.isLeapYear(expectedProlepticYear), isLeapYear) ;
        assertEquals(MinguoChronology.INSTANCE.isLeapYear(expectedProlepticYear), Year.of(expectedProlepticYear + YDIFF).isLeap()) ;
    }

    //-----------------------------------------------------------------------
    // with(DateTimeAdjuster)
    //-----------------------------------------------------------------------
    @Test(groups={"tck"})
    public void test_adjust1() {
        MinguoDate base = MinguoChronology.INSTANCE.date(2012, 10, 29);
        MinguoDate test = base.with(Adjusters.lastDayOfMonth());
        assertEquals(test, MinguoChronology.INSTANCE.date(2012, 10, 31));
    }

    @Test(groups={"tck"})
    public void test_adjust2() {
        MinguoDate base = MinguoChronology.INSTANCE.date(1728, 12, 2);
        MinguoDate test = base.with(Adjusters.lastDayOfMonth());
        assertEquals(test, MinguoChronology.INSTANCE.date(1728, 12, 31));
    }

    //-----------------------------------------------------------------------
    // MinguoDate.with(Local*)
    //-----------------------------------------------------------------------
    @Test(groups={"tck"})
    public void test_adjust_toLocalDate() {
        MinguoDate minguo = MinguoChronology.INSTANCE.date(99, 1, 4);
        MinguoDate test = minguo.with(LocalDate.of(2012, 7, 6));
        assertEquals(test, MinguoChronology.INSTANCE.date(101, 7, 6));
    }

    @Test(groups={"tck"}, expectedExceptions=DateTimeException.class)
    public void test_adjust_toMonth() {
        MinguoDate minguo = MinguoChronology.INSTANCE.date(1726, 1, 4);
        minguo.with(Month.APRIL);
    }

    //-----------------------------------------------------------------------
    // LocalDate.with(MinguoDate)
    //-----------------------------------------------------------------------
    @Test(groups={"tck"})
    public void test_LocalDate_adjustToMinguoDate() {
        MinguoDate minguo = MinguoChronology.INSTANCE.date(101, 10, 29);
        LocalDate test = LocalDate.MIN.with(minguo);
        assertEquals(test, LocalDate.of(2012, 10, 29));
    }

    @Test(groups={"tck"})
    public void test_LocalDateTime_adjustToMinguoDate() {
        MinguoDate minguo = MinguoChronology.INSTANCE.date(101, 10, 29);
        LocalDateTime test = LocalDateTime.MIN.with(minguo);
        assertEquals(test, LocalDateTime.of(2012, 10, 29, 0, 0));
    }

    //-----------------------------------------------------------------------
    // toString()
    //-----------------------------------------------------------------------
    @DataProvider(name="toString")
    Object[][] data_toString() {
        return new Object[][] {
            {MinguoChronology.INSTANCE.date(1, 1, 1), "Minguo ROC 1-01-01"},
            {MinguoChronology.INSTANCE.date(1728, 10, 28), "Minguo ROC 1728-10-28"},
            {MinguoChronology.INSTANCE.date(1728, 10, 29), "Minguo ROC 1728-10-29"},
            {MinguoChronology.INSTANCE.date(1727, 12, 5), "Minguo ROC 1727-12-05"},
            {MinguoChronology.INSTANCE.date(1727, 12, 6), "Minguo ROC 1727-12-06"},
        };
    }

    @Test(dataProvider="toString", groups={"tck"})
    public void test_toString(MinguoDate minguo, String expected) {
        assertEquals(minguo.toString(), expected);
    }

    //-----------------------------------------------------------------------
    // equals()
    //-----------------------------------------------------------------------
    @Test(groups="tck")
    public void test_equals_true() {
        assertTrue(MinguoChronology.INSTANCE.equals(MinguoChronology.INSTANCE));
    }

    @Test(groups="tck")
    public void test_equals_false() {
        assertFalse(MinguoChronology.INSTANCE.equals(IsoChronology.INSTANCE));
    }

}
