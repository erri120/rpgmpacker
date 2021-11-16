interface StackItem<T> {
  Previous: StackItem<T> | null,
  Next: StackItem<T> | null,
  Value: T
}

export default class Stack<T> {
  private _top: StackItem<T> | null;
  private _size: number;

  constructor() {
    this._top = null;
    this._size = 0;
  }

  push(value: T) {
    const item: StackItem<T> = {
      Previous: null,
      Next: null,
      Value: value
    };

    if (this._size === 0) {
      this._top = item;
    } else {
      item.Previous = this._top;
      this._top = item;
    }

    this._size += 1;
  }

  public get size(): number { return this._size; }

  public peak(): T | null {
    return this._top === null ? null : this._top.Value;
  }

  public pop() : T {
    if (this._top === null) {
      throw new Error("No items in Stack!");
    }

    const item = this._top;
    const prev = item.Previous;

    if (prev !== null) {
      prev.Next = null;
    }

    this._top = prev;
    this._size -= 1;
    return item.Value;
  }
}