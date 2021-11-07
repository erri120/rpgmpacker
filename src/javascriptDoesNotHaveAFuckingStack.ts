interface StackItem<T> {
  Previous?: StackItem<T>,
  Next?: StackItem<T>,
  Value: T
}

export class Stack<T> {
  private top: StackItem<T> | undefined;
  private count: number;

  constructor() {
    this.count = 0;
  }

  push(value: T) {
    const item: StackItem<T> = {
      Value: value
    };

    if (this.count === 0) {
      this.top = item;
    } else {
      item.Previous = this.top;
      this.top = item;
    }

    this.count += 1;
  }

  get size(): number {
    return this.count;
  }

  get peak(): T | undefined {
    return this.top?.Value;
  }

  get pop(): T {
    if (this.top === undefined) {
      throw new Error("No items in Stack!");
    }

    const item = this.top;
    const prev = item.Previous;
    if (prev) {
      prev.Next = undefined;
    }

    this.top = prev;
    this.count -= 1;
    return item.Value;
  }
}