// http://stackoverflow.com/a/18137117
struct Aligned {
	static void*
	operator new(size_t sz) {
		const size_t extra = sizeof(void *);
		void *mem = new char[sz + extra + 15];
		void *aligned = (void *)(
		    ((uintptr_t)(mem) + extra + 15) & ~15
		);
		void **ptr = (void **)(aligned);
		ptr[-1] = mem;
		return aligned;
	}

	static void
	operator delete(void* aligned, size_t sz)
	{
		void **ptr = (void **) aligned;
		char *mem = (char *)(ptr[-1]);
		delete[] mem;
	}
};
