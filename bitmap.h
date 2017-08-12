#ifndef BITMAP_H_
#define BITMAP_H_

#include "vajolet.h"
#include <string>

class bitmap2
{
private:
	unsigned long long b;
public:
	inline tSquare firstOne(void) const
	{
		assert(b);
		return ((tSquare)__builtin_ctzll(b));
	}


	bitmap2(){};
	bitmap2(bitMap _b): b(_b){}
	bitmap2(const bitmap2& x): b(x.b){}
	std::string displayBitmap(void)const;

	/* iterator */
	class iterator: public std::iterator<
	                        std::input_iterator_tag,   // iterator_category
							tSquare,                      // value_type
							tSquare,
							const tSquare*,
							tSquare
							>{
		unsigned long long b;
		public:
			explicit iterator(unsigned long long _b = 0ull) : b(_b) {}
			iterator& operator++() { b &= ( b - 1 ); return *this;}
			iterator operator++(int) { iterator retval = *this; ++(*this); return retval;}
			bool operator==(iterator other) const { return b == other.b; }
			bool operator!=(iterator other) const { return b != other.b; }
			reference operator*() const {return (tSquare)__builtin_ctzll(b);}
	};
	iterator begin() {return iterator(b);}
	iterator end() {return iterator(0);}

	/* operators */
	//inline bitmap2 operator += (const tSquare sq) { b |= bitHelper::getBitmapFromSquare(sq); return *this; }
	inline bitmap2& operator=(const bitmap2& other){ b = other.b; return *this;}

//	inline bool operator=(const bitmap2& other){ return other.b;}
	inline bitmap2& operator|=(const bitmap2& other){ b |= other.b; return *this;}
	inline bitmap2& operator&=(const bitmap2& other){ b &= other.b; return *this;}
	inline bitmap2& operator^=(const bitmap2& other){ b ^= other.b; return *this;}
	inline bitmap2 operator|(const bitmap2& other) const { return bitmap2(b | other.b); }
	inline bitmap2 operator&(const bitmap2& other) const { return bitmap2(b & other.b); }
	inline bitmap2 operator^(const bitmap2& other) const { return bitmap2(b ^ other.b); }
	inline bitmap2 operator&&(const bitmap2& other) const { return bitmap2(b && other.b); }

	inline bitmap2& operator|=(const bitMap& other){ b |= other; return *this;} // todo remove
	inline bitmap2& operator&=(const bitMap& other){ b &= other; return *this;} // todo remove
	inline bitmap2 operator|(const bitMap& other) const { return bitmap2(b | other); } // todo remove
	inline bitmap2 operator&(const bitMap& other) const { return bitmap2(b & other); } // todo remove
	inline bitmap2& operator^=(const bitMap& other){ b ^= other; return *this;} // todo remove
	inline bitMap getBitmap(void)const { return b;} // TODO remove
	inline bitmap2& operator=(const bitMap& other){ b = other; return *this;} // todo remove

	inline operator !() const { return b==0;}
	inline operator bool() const { return b!=0;}
	inline operator bitMap() const { return b;}


};

#endif
