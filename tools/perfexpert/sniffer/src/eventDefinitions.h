
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
