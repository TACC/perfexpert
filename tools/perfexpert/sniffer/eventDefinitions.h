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
 * along with PerfExpert.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ashay Rane
 */

#ifndef	EVENT_DEFINITIONS_H_
#define	EVENT_DEFINITIONS_H_

#define	BOOL			unsigned char
#define	EVENT_NOT_PRESENT	((BOOL) 0)
#define	EVENT_PRESENT		((BOOL) 1)
#define	EVENT_USED		((BOOL) 2)
#define	EVENT_ADDED		((BOOL) 3)

#define	USE_EVENT(X)		(event_list[X].use = EVENT_USED)
#define	ADD_EVENT(X)		(event_list[X].use = EVENT_ADDED)
#define	DOWNGRADE_EVENT(X)	(event_list[X].use = EVENT_PRESENT)

#define	IS_EVENT_AVAILABLE(X)	(X < EVENT_COUNT && event_list[X].use != EVENT_NOT_PRESENT)
#define	IS_EVENT_USED(X)	(X < EVENT_COUNT && event_list[X].use == EVENT_USED)
#define	IS_EVENT_ADDED(X)	(X < EVENT_COUNT && event_list[X].use == EVENT_ADDED)

#endif	/* EVENT_DEFINITIONS_H_ */
