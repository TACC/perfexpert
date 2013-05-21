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

package edu.utexas.tacc.argp.options;

import edu.utexas.tacc.argp.options.ArgType;

public class FlaggedOption extends AOption implements INamedOption,
                                                      IValuedOption {
    ArgType type;
    Character shortName;
    boolean mandatoryValue;
    String longName, valueName;

    public FlaggedOption(Character shortName, String longName,
                         String description, String valueName,
                         boolean mandatoryValue, ArgType type) {
        super(description);

        if (null != shortName || null != longName) {
            this.type = type;
            this.longName = longName;
            this.shortName = shortName;
            this.valueName = valueName;
            this.mandatoryValue = mandatoryValue;
        }
    }

    public Character getShortName() {
        return shortName;
    }

    public String getLongName() {
        return longName;
    }

    public ArgType getType() {
        return type;
    }

    public boolean takesValue() {
        return true;
    }

    public String getValueName() {
        return valueName;
    }

    public boolean isValueMandatory() {
        return mandatoryValue;
    }
}
