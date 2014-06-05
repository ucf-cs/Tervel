#ifndef TERVEL_WFHM_KEY_H
#define TERVEL_WFHM_KEY_H

template <class T>
class Key{
public: 
	explicit Key<T>(T key)
		: key_(key) {}

	~Key<T>() {}

	Key<T> get_key(){

	}
	void get_hash(){

	}
	void get_index(depth){

	}
	bool Key::operator==(const Key &other) const {
		return (*this == other);
	}
	bool Key::operator!=(const Key &other) const {
		return !(*this == other);
	}
private:
	T key_;
};
