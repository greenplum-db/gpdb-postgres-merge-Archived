/*-------------------------------------------------------------------------
 *
 * sdir.h
 *	  POSTGRES scan direction definitions.
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/sdir.h,v 1.21 2009/01/01 17:23:56 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/sdir.h,v 1.20 2008/01/01 19:45:56 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */
#ifndef SDIR_H
#define SDIR_H


/*
 * ScanDirection was an int8 for no apparent reason. I kept the original
 * values because I'm not sure if I'll break anything otherwise.  -ay 2/95
 */
typedef enum ScanDirection
{
	BackwardScanDirection = -1,
	NoMovementScanDirection = 0,
	ForwardScanDirection = 1
} ScanDirection;

/*
 * ScanDirectionIsValid
 *		True iff scan direction is valid.
 */
#define ScanDirectionIsValid(direction) \
	((bool) (BackwardScanDirection <= (direction) && \
			 (direction) <= ForwardScanDirection))

/*
 * ScanDirectionIsBackward
 *		True iff scan direction is backward.
 */
#define ScanDirectionIsBackward(direction) \
	((bool) ((direction) == BackwardScanDirection))

/*
 * ScanDirectionIsNoMovement
 *		True iff scan direction indicates no movement.
 */
#define ScanDirectionIsNoMovement(direction) \
	((bool) ((direction) == NoMovementScanDirection))

/*
 * ScanDirectionIsForward
 *		True iff scan direction is forward.
 */
#define ScanDirectionIsForward(direction) \
	((bool) ((direction) == ForwardScanDirection))

#endif   /* SDIR_H */
