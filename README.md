#+TITLE: ArduinoExpress
#+AUTHOR: Daniel Adejumo
#+EMAIL: adejumodaniel17@gmail.com

* Library Information
  - Name :: ArduinoExpress
  - Version :: 1.0.0
  - License :: BSD
  - URL :: https://github.com/danieladejumo17/ArduinoExpress
  - Author :: Daniel Adejumo
  - Email :: adejumodaniel17@gmail.com

This is a lightweight implementation of the popular Express web framework in C++ for MCU boards.

  A sequence container similar to the C++
  [[http://www.cplusplus.com/reference/vector/vector/][std::vector]], but
  instead of allocating memory dynamically, this container points to an
  external, statically allocated c style array. The maximum size is
  fixed at compile time, but the size can change by pushing and popping
  elements from the vector. Static memory allocation is used to avoid
  dynamic allocation problems on very small embedded processors. Care
  must be taken not to dereference an empty vector, access elements
  beyond bounds, or use without setting the storage array.

  This library is very similar to
  [[https://github.com/janelia-arduino/Array][Array]], however Array
  stores data internally in the container and this library stores data
  externally. The pointer to the external memory causes this container
  to use more memory than the Array container, but storing the data
  externally avoids needing the maximum size as a class template
  parameter.

* Vector vs Array
** Vector

   #+BEGIN_SRC C++
     const int ELEMENT_COUNT_MAX = 5;
     int storage_array[ELEMENT_COUNT_MAX];
     Vector<int> vector(storage_array);
     vector.push_back(77);
   #+END_SRC

** Array

   #+BEGIN_SRC C++
     const int ELEMENT_COUNT_MAX = 5;
     Array<int,ELEMENT_COUNT_MAX> array;
     array.push_back(77);
   #+END_SRC
