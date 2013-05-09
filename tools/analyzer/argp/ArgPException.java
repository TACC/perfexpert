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

package edu.utexas.tacc.argp;

import java.text.ParseException;

public class ArgPException extends ParseException {
    // XXX: Don't know if this will completely fix the problem
    // although it makes the warning go away

    public static final long serialVersionUID = 24362462L;
    public ArgPException(String msg) {
        super(msg, 0);
    }

    public ArgPException(String msg, int offset) {
        super(msg, offset);
    }
}
