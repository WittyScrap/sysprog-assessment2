#define SIGN(x)     (((x) >= 0) - ((x) < 0))                        // The sign of an integer or floating point number.
#define MASK(x)     (-(!!(x)))                                      // Generates a mask that is all 1s if the condition is true and all 0s if it is not
#define ABS(x)      ((x) * (MASK((x) < 0) | 1))                     // Retrieves the absolute value of a number
#define MIN(x, m)   (((x) * ((x) < (m))) + ((m) * ((x) >= (m))))    // Retrieves the mininmum between two numbers
#define MAX(x, m)   (((x) * ((x) > (m))) + ((m) * ((x) <= (m))))    // Retrieves the maximum between two numbers