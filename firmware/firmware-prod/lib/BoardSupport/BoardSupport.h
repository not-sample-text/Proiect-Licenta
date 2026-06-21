#pragma once

/**
 * @brief Handles core hardware initialization.
 */
class BoardSupport {
public:
    /**
     * @brief Initializes critical board resources (Serial, I2C, Power Rail).
     */
    static void begin();
};
