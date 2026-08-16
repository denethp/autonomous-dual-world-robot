long distToCounts(float dist_mm) {
  return (long)((dist_mm / (PI * WHEEL_DIAMETER)) * CPR);
}

float countsToDist(long counts) {
  return ((float)counts * (PI * WHEEL_DIAMETER)) / CPR;
}