#pragma once

void hidInit();
void hidFlush();

bool hidIsButtonPressed(int button);
bool hidIsButtonDown(int button);
bool hidIsButtonUp(int button);

float hidGetAxis(int axis);