// src/SafeVar.h - VERSION ESP32 PURE
#ifndef SAFEVAR_H
#define SAFEVAR_H

#include <Arduino.h>

template<typename T>
class SafeVar {
private:
  mutable portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  volatile T value;

public:
  SafeVar(T initialValue = T()) : value(initialValue) {}
  
  T get() const {
    portENTER_CRITICAL(&mux);
    T temp = value;
    portEXIT_CRITICAL(&mux);
    return temp;
  }
  
  void set(T newValue) {
    portENTER_CRITICAL(&mux);
    value = newValue;
    portEXIT_CRITICAL(&mux);
  }
  
  operator T() const { return get(); }
  
  SafeVar& operator=(T newValue) {
    set(newValue);
    return *this;
  }
  
  SafeVar& operator+=(T val) {
    set(get() + val);
    return *this;
  }
  
  SafeVar& operator-=(T val) {
    set(get() - val);
    return *this;
  }
};

#endif