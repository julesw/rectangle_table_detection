#ifndef _UNIQUE_LOOPS_MNGR_
#define _UNIQUE_LOOPS_MNGR_
#include <iostream>
#include <vector>
/** \class UniqueLoopsMngr
 * Manages a list of arrays considered as bidirectionnal looped lists (circuits) so that they are unique.
 * The loops are std::vector<T*> and the comparison between two loops is made so that 2 loops with same
 * elements in same order (direct or inverse) are considered equals.
 * i.e. [A B C D] is considered equal to [C B A D] and [C D A B]
 *
 * \author Jules Waldhart
 */
template <class T>
class UniqueLoopsMngr{

	public:
		UniqueLoopsMngr();

	public:
		std::vector<std::vector<T> > _loops_collection;

	public:
		bool addLoop(const std::vector<T>& loop);
		bool loopAlreadyExists(const std::vector<T>& loop);
		bool compareLoops(const std::vector<T>& loop1,const std::vector<T>& loop2);


};

#endif // _UNIQUE_LOOPS_MNGR_
#include "UniqueLoopsMngr.tpp"
