

template<class T>
class EraseAt : public OpRecord{
typedef ShiftHelper< EraseAt<T> > helper_t
public:

  const size_t idx_;
  const Vector<T> *vector_;
  std::atomic<bool> is_complete_;
  std::atomic<helper_t *> next_;


  EraseAt(const Vector *vector, const size_t idx)
    : idx_(idx)
    , vector_(vector)
    , is_complete_(false)
    , next_(nullptr) {};

  bool begin(const T &result) {
    execute();

    if (next_.load() != reinterpret_cast<helper_t *>(0x1)) {
        result = next_.load()->rvalue;
        return true;
    } else {
        return false;
    }
  };

  void execute() {
    tl_control_word=(std::atomic<void *> *)&(this->is_complete_);
    complete(vector_, idx_);
  };

  void cleanup();

  bool complete(int pos) {
      return completeShift< EraseAt<T> >(this, vector_, pos);
  };

  bool is_complete() {
    return is_complete_.load();
  }
};

template<class T>
void EraseAt::cleanup() {
  helper_t *parent = next;
  helper_t *helper = parent->next.load();
  int i;

  for (i = idx_+1; helper != NULL; i++) {
    ArrayElement *spot = vec->getSpot(i);
    T current = spot->load();

    T unmark_helper =
                    reinterpret_cast<T>(util::memory::rc::unmark_first(helper));
    if (current == unmark_helper) {
      if (helper->next.load() == NULL) {
        spot->compare_exchange_strong(current, Vector::c_not_value_);
      } else {
        spot->compare_exchange_strong(current, helper->next.load()->rvalue);
      }
    }
    parent = helper;
    helper = helper->next.load();
  }
};
