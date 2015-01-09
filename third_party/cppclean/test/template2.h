template <>
class ErrorOr<void> {

  ~ErrorOr() {
    getPointer()->release();
  }

};
