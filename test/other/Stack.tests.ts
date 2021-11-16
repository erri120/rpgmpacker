import { describe, it } from "mocha";
import { expect } from "chai";

import { Stack } from "../../src/other/Stack";

describe("Stack", () => {
  it("push peak pop", () => {
    const stack = new Stack<number>();
    expect(stack.size).to.equal(0);
    expect(stack.peak()).to.be.null;

    stack.push(1);
    expect(stack.size).to.equal(1);
    expect(stack.peak()).to.equal(1);

    const val = stack.pop();
    expect(val).to.equal(1);
    expect(stack.size).to.equal(0);
    expect(stack.peak()).to.be.null;
  });
});