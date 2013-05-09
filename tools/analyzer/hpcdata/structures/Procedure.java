/*
 * Copyright (C) 2013 The University of Texas at Austin
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ashay Rane
 */

package edu.utexas.tacc.hpcdata.structures;

public class Procedure extends Node implements Comparable<Procedure> {
    private long s, l, n, lm, f, a;
    
    public Procedure() {
        s  = 0;
        l  = 0;
        n  = 0;
        lm = 0;
        f  = 0;
        a  = 0;
    }
    
    public Procedure(long i, long s, long l, long n, long lm, long f) {
        super();
        
        type = "P";
        this.i = i;
        this.s = s;
        this.l = l;
        this.n = n;
        this.lm = lm;
        this.f = f;
    }
    
    public Procedure(long i, long s, long l, long n, long lm, long f, long a) {
        this(i, s, l, n, lm, f);
        
        this.a = a;
    }
    
    public Procedure(String i, String s, String l, String n, String lm,
                     String f) {
        super();
        
        type = "P";
        if (null != i) {
            this.i = Long.parseLong(i);
        }
        if (null != s) {
            this.s = Long.parseLong(s);
        }
        if (null != l) {
            this.l = Long.parseLong(l);
        }
        if (null != n) {
            this.n = Long.parseLong(n);
        }
        if (null != lm) {
            this.lm = Long.parseLong(lm);
        }
        if (null != f) {
            this.f = Long.parseLong(f);
        }
    }
    
    public Procedure(String i, String s, String l, String n, String lm,
                     String f, String a) {
        this(i, s, l, n, lm, f);
        
        if (null != a) {
            this.a = Long.parseLong(a);
        }
    }
    
    public long getL() {
        return l;
    }
    
    public long getS() {
        return s;
    }
    
    public long getN() {
        return n;
    }
    
    public long getLM() {
        return lm;
    }
    
    public long getF() {
        return f;
    }
    
    public long getA() {
        return a;
    }
    
    public int compareTo(Procedure p) {
        if (this.lm != p.lm) {
            return 2 * ((int) (this.lm-p.lm)) / ((int) abs(this.lm-p.lm));
        }
        if (this.f != p.f) {
            return ((int) (this.f - p.f)) / ((int) abs(this.f - p.f));
        }
        return 0;
    }
    
    private long abs(long x) {
        if (x < 0) {
            return -x;
        }
        return x;
    }
    
    public String toString() {
        return "Procedure #"+i;
    }
}
