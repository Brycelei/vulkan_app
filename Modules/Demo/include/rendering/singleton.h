#ifndef SINGLETON_H
#define SINGLETON_H

class Singleton {
public:

	// Delete the copy constructor and assignment operator to prevent copying
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	// Delete the move constructor and assignment operator to prevent moving
	Singleton(const Singleton&&) = delete;
	Singleton& operator=(const Singleton&&) = delete;
	
	// Provide a static method to get the instance of the singleton
	static Singleton& GetInstance() {
		static Singleton instance; // Guaranteed to be destroyed, instantiated on first use
		return instance;
	}

protected:
	// Protected constructor to prevent instantiation from outside
	Singleton() = default;
	virtual ~Singleton() = default;
};

#endif