/*
 * Timeline class, which implements an adaptive timeline for a graph.
 * Given two dates, it will return a list containing the labels to draw 
 * and where they should be drawn, having worked out what scale it is 
 * reasonable to display at.
 *
 * Nigel Stuckey, 1999
 * Copyright System Garden Ltd 1999-2001. All rights reserved.
 */


#include <limits.h>
#include <string.h>
#include "timeline.h"
#include "nmalloc.h"

struct timeline_tickpointdefs {
     int threshold;			/* upper bound in secs */
     enum timeline_units base;		/* base point unit */
     int major;				/* secs in major tick */
     int minor;				/* secs in minor tick */
     char *description;			/* tick unit text description */
} timeline_tickpoints[] = {
     {120,      SECS,     10,       1,      "seconds"},	/* < 2 minutes */
     {600,      MINS,     60,       10,     "minutes"},	/* < 10 minutes */
     {3600,     MINS,     300,      60,     "minutes"},	/* < 1 hour */
     {7200,     MINS,     600,      300,    "minutes"},	/* < 2 hours */
     {25200,    MINS,     1800,     300,    "hours"},	/* < 7 hours */
     {50400,    HOURS,    3600,     600,    "hours"},	/* < 14 hours */
     {129600,   HOURS,    7200,     600,    "hours"},	/* < 1.5 days */
     {604800,   WEEKDAYS, 86400,    3600,   "weekdays"},/* < 1 week */
     {1209600,  DAYS,     86400,    43200,  "days"},	/* < 2 weeks */
     {2678400,  MDAYS,    86400,    43200,  "days"},	/* < 1 month */
     {8035200,  MONTHS,   2678400,  86400,  "months"},	/* < 3 months */
     {36892800, MONTHS,   2678400,  669600, "months"},	/* < 14 months */
     {INT_MAX,  YEARS,    31536000, 2678400,"years"},	/* > 1 year */
     {0,        NOUNIT,   0,       0,     NULL}
};
time_t timeline_offset = 0;



/*
 * Set an offset for time lines genrated by timeline_calc().
 * New offsets replace replace previous ones, rather than add to them.
 */
void timeline_setoffset(time_t offset)
{
     timeline_offset = offset;
}


/*
 * Return the actual time given point which is an offset from 
 * timeline_setoffset()
 */
time_t timeline_actual(time_t point)
{
     return point + timeline_offset;
}

/*
 * Return the label representing the time at point.
 * Point is an offset in seconds to the value provided to timeline_offset
 * so the time returned will be point+timeline_offset.
 * Scope is the context which dictates the accuracy and form of the label
 * and is in the form of the number of seconds covered by this timeline.
 * The formatted time string will copied into char array label of length 
 * in size which should be at least TIMELINE_SHORTSTR in length to avoid 
 * truncation
 * Returns a pointer to the first char of the array of NULL if there was an
 * error.
 */
char * timeline_label(time_t point, time_t scope, char *label, int length)
{
     time_t t;				/* working time */
     struct tm  t_tm;			/* working time breakdown */

     t = timeline_actual(point);
     localtime_r(&t, &t_tm);

     if ( ! label)
       return NULL;

     /*printf("scope %d\n", scope);*/

     if (scope < 600)		/* < 10 mins */
       strftime(label, length, "%H:%M:%S", &t_tm);
     else if (scope < 25200)	/* < 7 hours */
       strftime(label, length, "%H:%M", &t_tm);
     else if (scope < 129600)	/* < 1.5 days */
       strftime(label, length, "%H:00", &t_tm);
     else if (scope < 604800)	/* < 1 week */
       strftime(label, length, "%a", &t_tm);
     else if (scope < 1209600)	/* < 2 weeks */
       strftime(label, length, "%d%b", &t_tm);
     else if (scope < 5356800)	/* < 2 months */
       strftime(label, length, "%d%b", &t_tm);
     else if (scope < 21427200)	/* < 8 months */
       strftime(label, length, "%d%b%y", &t_tm);
     else if (scope < 36892800)	/* < 14 months */
       strftime(label, length, "%b%y", &t_tm);
     else
       strftime(label, length, "%Y", &t_tm);

     return label;
}



#include "../iiab/util.h"
#include <stdio.h>
/*
 * Calculate the major and minor x-axis points of a time line, given the
 * line size and viewing scale.
 * Dispdiff is a representation of the displayed scale and should be 
 * ( <upper time> - <lower time> ), giving the number seconds being 
 * displayed at once.
 * Min and max times have an offset added as set by timeline_setoffset(). 
 * Results are returned in an ITREE list, with the key being time_t
 * and the value being a pointer to a struct timeline_tick.
 * The value node indicates the type of tick it is and the text label
 * associated with it.
 * Returns NULL for an error, such as being impossible to work out.
 */
ITREE *timeline_calc(time_t min,	/* start of the timeline */
		     time_t max,	/* end of timeline */
		     time_t dispdiff,	/* viewing area for scale */
		     int maxchars	/* max number of chars possible */)
{
     int diff;				/* diff betwix min+max */
     int maxlabels;			/* max number of labels poss */
     int tickindex;			/* index to applicable tick point */
     ITREE *results;			/* returned results */
     time_t t;				/* working time */
     struct tm  t_tm;			/* working time breakdown */
     struct timeline_tick *tick;	/* tick details */
     char label[TIMELINE_SHORTSTR];	/* working label text */

     fprintf(stderr, "min %ld (%s), max %ld (%s), diff %ld, maxchars %d\n", 
	     min, util_decdatetime(min+timeline_offset), 
	     max, util_sdecdatetime(max+timeline_offset),
	     dispdiff, maxchars);

     /* we must have at least 3 seconds to work sensibly */
     diff = max - min;
     if (min +3 >= max || diff < 3)
	  return NULL;

     /* work out the maximum number of labels possible, if the widest
      * is 8 chars + 1 for the major tick + 1 space */
     maxlabels = maxchars / (1+8+1);

     results = itree_create();

     /* find which tick is applicable from the table */
     for (tickindex = 0; timeline_tickpoints[ tickindex ].threshold; 
			tickindex++)
	  if (dispdiff <= timeline_tickpoints[ tickindex ].threshold)
	       break;
     fprintf(stderr, "tkidx %d, thresh %d, base %d, maj %d, min %d, desc %s\n",
	     tickindex, timeline_tickpoints[ tickindex ].threshold,
	     timeline_tickpoints[ tickindex ].base,
	     timeline_tickpoints[ tickindex ].major,
	     timeline_tickpoints[ tickindex ].minor,
	     timeline_tickpoints[ tickindex ].description);

     /* Start point should be the DATE of the min as a non-tick. Store it */
     t = min + timeline_offset;
     localtime_r(&t, &t_tm);
     strftime(label, TIMELINE_SHORTSTR, "%d%b%y", &t_tm);
     tick = nmalloc(sizeof(struct timeline_tick));
     tick->type  = TIMELINE_NONE;
     tick->label = xnstrdup(label);
     tick->point = min;
     tick->ptime = t;
     itree_add(results, tick->point, tick);
     fprintf(stderr, "  point %ld (%s), type %d, label %s\n", 
	     tick->point, util_decdatetime(t), 
	     tick->type, tick->label);

     /* Min and max are the visible limits of the chart. 
      * The counting, however, needs to start from the greatest major 
      * point lower than the min */
     t -= timeline_tickpoints[ tickindex ].major;

     /* establish the first rounded time before looping */
     timeline_rounddown(&t_tm, timeline_tickpoints[ tickindex ].base);
     t = mktime(&t_tm);

     /* run off major ticks */
     while (t < max + timeline_offset) {
	  /* add major tick increment */
	  t += timeline_tickpoints[ tickindex ].major;
	  localtime_r(&t, &t_tm);

	  /* create label text */
	  switch (timeline_tickpoints[ tickindex ].base) {
	  case NOUNIT:
	       strftime(label, TIMELINE_SHORTSTR, "??", &t_tm);
	       break;
	  case SECS:
	       strftime(label, TIMELINE_SHORTSTR, "%H:%M:%S", &t_tm);
	       break;
	  case MINS:
	       strftime(label, TIMELINE_SHORTSTR, "%H:%M", &t_tm);
	       break;
	  case HOURS:
	       strftime(label, TIMELINE_SHORTSTR, "%H:00", &t_tm);
	       break;
	  case WEEKDAYS:
	       strftime(label, TIMELINE_SHORTSTR, "%a", &t_tm);
	       break;
	  case DAYS:
	       strftime(label, TIMELINE_SHORTSTR, "%d%b", &t_tm);
	       break;
	  case MDAYS:
	       strftime(label, TIMELINE_SHORTSTR, "%d", &t_tm);
	       break;
	  case MONTHS:
	       strftime(label, TIMELINE_SHORTSTR, "%b", &t_tm);
	       break;
	  case YEARS:
	       strftime(label, TIMELINE_SHORTSTR, "%Y", &t_tm);
	       break;
	  }

	  /* store tick as a result */
	  tick = nmalloc(sizeof(struct timeline_tick));
	  tick->type  = TIMELINE_MAJOR;
	  tick->label = xnstrdup(label);
	  tick->point = t - timeline_offset;
	  tick->ptime = t;
	  itree_add(results, tick->point, tick); /* really need an itree? */
	  fprintf(stderr, "  point %ld (%s), type %d, label %s\n", 
		  tick->point, util_decdatetime(t), 
		  tick->type, tick->label);
     }

     /* run off minor ticks */
     t = min + timeline_offset;

     /* As major points, start counting from the greatest minor
      * point lower than the min */
     t -= timeline_tickpoints[ tickindex ].minor;
     localtime_r(&t, &t_tm);

     /* establish the first rounded time before looping */
     timeline_rounddown(&t_tm, timeline_tickpoints[ tickindex ].base);
     t = mktime(&t_tm);

     while (t < max + timeline_offset) {
	  /* add minor tick increment */
	  t += timeline_tickpoints[ tickindex ].minor;
	  localtime_r(&t, &t_tm);

	  /* store tick as a result if there is not a major tick already */
	  if (itree_find(results, t) == ITREE_NOVAL) {
	       tick = nmalloc(sizeof(struct timeline_tick));
	       tick->type  = TIMELINE_MINOR;
	       tick->label = NULL;
	       tick->point = t - timeline_offset;
	       tick->ptime = t;
	       itree_add(results, tick->point, tick);
	       fprintf(stderr, "  point %ld (%s), type %d, label %s\n", 
		       tick->point, util_decdatetime(t), 
		       tick->type, tick->label);
	  }
     }

     /* max point (non-tick) */
     t = max + timeline_offset;
     localtime_r(&t, &t_tm);
     strftime(label, TIMELINE_SHORTSTR, "%d%b%y", &t_tm);
     /* store non-tick! as a result */
     tick = nmalloc(sizeof(struct timeline_tick));
     tick->type  = TIMELINE_NONE;
     tick->label = xnstrdup(label);
     tick->point = max;
     tick->ptime = t;
     itree_add(results, max, tick);
     fprintf(stderr, "  point %ld (%s), type %d, label %s\n", 
	     tick->point, util_decdatetime(t), 
	     tick->type, tick->label);

     return results;
}

/*
 * Calculate the major and minor x-axis points of a time line, given the
 * line size and viewing scale.
 * Dispdiff is a representation of the displayed scale and should be 
 * ( <upper time> - <lower time> ), giving the number seconds being 
 * displayed at once.
 * Min and max times have an offset added as set by timeline_setoffset(). 
 * Results are returned in an ITREE list, with the key being time_t
 * and the value being a pointer to a struct timeline_tick.
 * The value node indicates the type of tick it is and the text label
 * associated with it.
 * Returns NULL for an error, such as being impossible to work out.
 */
ITREE *timeline_ocalc(time_t min,	/* start of the timeline */
		     time_t max,	/* end of timeline */
		     time_t dispdiff	/* viewing area for scale */ )
{
     int tickindex;			/* index to applicable tick point */
     ITREE *results;			/* returned results */
     time_t t;				/* working time */
     struct tm  t_tm;			/* working time breakdown */
     struct tm *t_tmp;			/* returned time breakdown pointer */
     struct timeline_tick *tick;	/* tick details */
     char label[TIMELINE_SHORTSTR];	/* working label text */

     /* we must have at least 3 seconds to work sensibly */
     if (min +3 >= max || dispdiff < 3)
	  return NULL;

     results = itree_create();

     /* find which tick is applicable */
     for (tickindex = 0; timeline_tickpoints[ tickindex ].threshold; 
			tickindex++)
	  if (dispdiff <= timeline_tickpoints[ tickindex ].threshold)
	       break;

     /* start point (non-tick) */
     t = min + timeline_offset;
     t_tmp = localtime(&t);
     strftime(label, TIMELINE_SHORTSTR, "%d%b%y", t_tmp);
     /* store non-tick! as a result */
     tick = nmalloc(sizeof(struct timeline_tick));
     tick->type  = TIMELINE_NONE;
     tick->label = xnstrdup(label);
     tick->point = min;
     itree_add(results, min, tick);

     /* run off major ticks */
     t = min + timeline_offset;
     while (1) {
	  /* add major tick increment */
	  t    += timeline_tickpoints[ tickindex ].major;
	  t_tmp = localtime(&t);
	  memcpy(&t_tm, t_tmp, sizeof(struct tm));

	  /* round down */
	  timeline_rounddown(&t_tm, timeline_tickpoints[ tickindex ].base);
	  t = mktime(&t_tm);

	  /* loop check */
	  if (t >= max + timeline_offset)
	       break;

	  /* create label text */
	  switch (timeline_tickpoints[ tickindex ].base) {
	  case NOUNIT:
	       strftime(label, TIMELINE_SHORTSTR, "??", &t_tm);
	       break;
	  case SECS:
	       strftime(label, TIMELINE_SHORTSTR, "%H:%M:%S", &t_tm);
	       break;
	  case MINS:
	       strftime(label, TIMELINE_SHORTSTR, "%H:%M", &t_tm);
	       break;
	  case HOURS:
	       strftime(label, TIMELINE_SHORTSTR, "%H:00", &t_tm);
	       break;
	  case WEEKDAYS:
	       strftime(label, TIMELINE_SHORTSTR, "%a", &t_tm);
	       break;
	  case DAYS:
	       strftime(label, TIMELINE_SHORTSTR, "%d%b", &t_tm);
	       break;
	  case MDAYS:
	       strftime(label, TIMELINE_SHORTSTR, "%d", &t_tm);
	       break;
	  case MONTHS:
	       strftime(label, TIMELINE_SHORTSTR, "%b", &t_tm);
	       break;
	  case YEARS:
	       strftime(label, TIMELINE_SHORTSTR, "%Y", &t_tm);
	       break;
	  }

	  /* store tick as a result */
	  tick = nmalloc(sizeof(struct timeline_tick));
	  tick->type  = TIMELINE_MAJOR;
	  tick->label = xnstrdup(label);
	  tick->point = t - timeline_offset;
	  itree_add(results, t - timeline_offset, tick);
     }

     /* run off minor ticks */
     t = min + timeline_offset;
     while (1) {
	  /* add maior tick increment */
	  t    += timeline_tickpoints[ tickindex ].minor;
#if 0
/* suggest that minor ticks do not need the rounding treatment */
	  t_tmp = localtime(&t);
	  memcpy(&t_tm, t_tmp, sizeof(struct tm));

	  /* round down */
	  timeline_rounddown(&t_tm, timeline_tickpoints[ tickindex ].base);
	  t = mktime(&t_tm);
printf("%d\n", t);
#endif
	  /* loop check */
	  if (t >= max + timeline_offset)
	       break;

	  /* store tick as a result if there is not a major tick already */
	  if (itree_find(results, t) == ITREE_NOVAL) {
	       tick = nmalloc(sizeof(struct timeline_tick));
	       tick->type  = TIMELINE_MINOR;
	       tick->label = NULL;
	       tick->point = t - timeline_offset;
	       itree_add(results, t - timeline_offset, tick);
	  }
     }

     /* max point (non-tick) */
     t = max + timeline_offset;
     t_tmp = localtime(&t);
     strftime(label, TIMELINE_SHORTSTR, "%d%b%y", t_tmp);
     /* store non-tick! as a result */
     tick = nmalloc(sizeof(struct timeline_tick));
     tick->type  = TIMELINE_NONE;
     tick->label = xnstrdup(label);
     tick->point = max;
     itree_add(results, max, tick);

     return results;
}

/* free data provided by timeline_calc(); the list argument will not 
 * be valid after this */
void timeline_free(ITREE *l)
{
     struct timeline_tick *tick;	/* tick details */

     while ( ! itree_empty(l) ) {
	  itree_first(l);
	  tick = itree_get(l);
	  if (tick->label)
	       nfree(tick->label);
	  nfree(tick);
	  itree_rm(l);
     }

     itree_destroy(l);
}


/*
 * Take a time breakdown (struct tm) and a timeline unit and round down
 * the time to an appropreate base unit.
 */
void timeline_rounddown(struct tm *t_tmp, enum timeline_units unit)
{
     switch (unit) {
     case NOUNIT:
	  break;
     case YEARS:
     case MONTHS:
	  t_tmp->tm_yday -= t_tmp->tm_mday-1;
	  t_tmp->tm_mday = 1;
	  t_tmp->tm_hour = 0;
     case MDAYS:
     case DAYS:
     case WEEKDAYS:
	  t_tmp->tm_hour = 0;
     case HOURS:
	  t_tmp->tm_min = 0;
     case MINS:
	  t_tmp->tm_sec = 0;
     case SECS:
	  break;
     }
}


/*
 * Take a time breakdown (struct tm) and a timeline unit and round down
 * the time to an appropreate base unit.
 */
void timeline_orounddown(struct tm *t_tmp, enum timeline_units unit)
{
     switch (unit) {
     case NOUNIT:
	  break;
     case YEARS:
     case MONTHS:
	  t_tmp->tm_yday -= t_tmp->tm_mday-1;
	  t_tmp->tm_mday = 1;
	  t_tmp->tm_hour = 0;
     case MDAYS:
     case DAYS:
     case WEEKDAYS:
	  t_tmp->tm_min = 0;
     case HOURS:
     case MINS:
	  t_tmp->tm_sec = 0;
     case SECS:
	  break;
     }
}



#if TEST

#include <stdio.h>
#include "elog.h"

int main(int argc, char **argv)
{
     ITREE *l1;

     /* test 1a, @60s ticks every second plus barriers */
     l1 = timeline_calc(0, 100000, 60);
     if (l1 == NULL)
	  elog_die(FATAL, "[1a] timeline cowardly returns NULL\n");
     if (itree_n(l1) != 100001)
	  elog_die(FATAL, "[1a] should be 100001 in tree not %d\n", 
		   itree_n(l1));
     timeline_free(l1);

     /* test 1b, @180s ticks every 10s + barriers */
     l1 = timeline_calc(0, 100000, 180);
     if (l1 == NULL)
	  elog_die(FATAL, "[1b] timeline cowardly returns NULL\n");
     if (itree_n(l1) != 10001)
	  elog_die(FATAL, "[1b] should be 10001 in tree not %d\n", 
		   itree_n(l1));
     timeline_free(l1);

     /* test 1c, @610s ticks every 60s + barriers */
     l1 = timeline_calc(0, 100000, 610);
     if (l1 == NULL)
	  elog_die(FATAL, "[1c] timeline cowardly returns NULL\n");
     if (itree_n(l1) != 1668)
	  elog_die(FATAL, "[1c] should be 1668 in tree not %d\n", 
		   itree_n(l1));
     timeline_free(l1);

     /* test 1d, @6000s (<2 hrs) ticks every 300s (5mins) + barriers */
     l1 = timeline_calc(0, 100000, 6000);
     if (l1 == NULL)
	  elog_die(FATAL, "[1d] timeline cowardly returns NULL\n");
     if (itree_n(l1) != 335)
	  elog_die(FATAL, "[1d] should be 335 in tree not %d\n", 
		   itree_n(l1));
     timeline_free(l1);

     /* test 1e, @9000s (<14 hrs) ticks every 600s (10 mins) + barriers */
     l1 = timeline_calc(0, 100000, 9000);
     if (l1 == NULL)
	  elog_die(FATAL, "[1e] timeline cowardly returns NULL\n");
     if (itree_n(l1) != 168)
	  elog_die(FATAL, "[1e] should be 168 in tree not %d\n", 
		   itree_n(l1));
     timeline_free(l1);

     /* test 1f, @60000s (<1.5 days) ticks every 600s (10 mins) + barriers */
     l1 = timeline_calc(0, 100000, 60000);
     if (l1 == NULL)
	  elog_die(FATAL, "[1f] timeline cowardly returns NULL\n");
     if (itree_n(l1) != 168)
	  elog_die(FATAL, "[1f] should be 168 in tree not %d\n", 
		   itree_n(l1));
     timeline_free(l1);

     /* test 1g, @200000s (<1 week) ticks every 3600s (1 hrs) + barriers */
     l1 = timeline_calc(0, 5000000, 200000);
     if (l1 == NULL)
	  elog_die(FATAL, "[1g] timeline cowardly returns NULL\n");
     if (itree_n(l1) != 1390)
	  elog_die(FATAL, "[1g] should be 1390 in tree not %d\n", 
		   itree_n(l1));
     timeline_free(l1);

     /* test 1h, @700000s (<2 weeks) ticks every 43200s (12 hrs) + barriers */
     l1 = timeline_calc(0, 50000000, 700000);
     if (l1 == NULL)
	  elog_die(FATAL, "[1h] timeline cowardly returns NULL\n");
     if (itree_n(l1) != 1159)
	  elog_die(FATAL, "[1h] should be 1159 in tree not %d\n", 
		   itree_n(l1));
     timeline_free(l1);

     /* test 1i, @2000000s (<2 month) ticks every 43200s (12 hrs) + barriers */
     l1 = timeline_calc(0, 50000000, 2000000);
     if (l1 == NULL)
	  elog_die(FATAL, "[1i] timeline cowardly returns NULL\n");
     if (itree_n(l1) != 1159)
	  elog_die(FATAL, "[1i] should be 1159 in tree not %d\n", itree_n(l1));
     timeline_free(l1);

     /* test 1j, @9,000,000s (<3 months) ticks every 86400 (1d), 2678400 
      * + barriers */
     l1 = timeline_calc(0, 50000000, 9000000);
     if (l1 == NULL)
	  elog_die(FATAL, "[1j] timeline cowardly returns NULL\n");
     if (itree_n(l1) != 95)
	  elog_die(FATAL, "[1j] should be 95 in tree not %d\n", itree_n(l1));
     timeline_free(l1);

#if 0
     itree_traverse(l1)
	  printf("%10d  %s  %d  %s\n", itree_getkey(l1), 
		 util_decdatetime(itree_getkey(l1)),
		 ((struct timeline_tick *)itree_get(l1))->type,
		 (((struct timeline_tick *)itree_get(l1))->label == NULL)?
	       "":((struct timeline_tick *)itree_get(l1))->label);
     printf("%d lines\n", itree_n(l1));
     timeline_free(l1);
#endif

     printf("test finished successfully\n");
     return 0;
}
#endif /* TEST */
