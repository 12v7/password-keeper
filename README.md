## Password Keeper

The long and complicated password is required for reliable offline data protection, but it is inconvenient for many users to remember and type it often. This device is a USB keyboard that stores a 30-character password and types it if the user enters the short PIN code.
The password is created from the passphrase entered by the user with a terminal in interactive mode. The device stores the password only in RAM, so it will be lost if the device is shut down. Incorrect PIN or timeout during entering the PIN also leads to an erased password.

### Setting up the device

The device is based on the [Raspberry Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/) board.

1. Connect the board to the PC. The new removable drive should appear.
2. Copy to this drive the file with the device firmware.
3. As long as the drive is automatically disconnected, the device is ready to use.

### Password creating

The device also contains a serial port.
To create the password, connect to it with a terminal application (for example [TeraTerm](https://teratermproject.github.io/)). And then:

1. Press `P`
2. Type pass phrase.
3. Press `Enter`.

The device generates the password and PIN from the phrase and salt and shows the PIN in the terminal.

### Enter PIN code

The PIN is a sequence of short `.` and long (>500ms) `-` presses on the device button.
Just after the correct PIN is entered, the device types the stored 30-character password and `Enter`.
Pay attention to choosing the proper field where to enter the password.
Enter the password **only** in the designated field.
If you, by mistake, enter it, for example, in the browser address line, it will be sent to the Internet and stored in the browser history in plain text.

### FAQ

#### Meaning of LED blinking

| LED State | Explanation |
|--|--|
| On | Password has not entered yet |
| Fast blinking | Terminal connected or PIN code is entering |
| Short blink every 8 s | Has password, wait for PIN |

#### How to restore password if the device is lost

To generate the password, the device identifier was used as a salt. Therefore, you should have it in case you lose your device. In the terminal, press 'S' to view the identifier. It is not a secret; feel free to write it down in a file or somewhere else. There is a special application: rescue.exe. Run it, enter the salt and passphrase, and it will generate the password.

#### Is it secure to use the device

Despite the lack of a security audit, the software's creator took every precaution to avoid unwanted access.
Reading the password from device memory is theoretically conceivable, but it is extremely difficult to obtain.
To eliminate access to the password, just shut the device down.
You can also start PIN code, and unless you finish it in 10 minutes, the password will be erased as well.
Read [here](https://en.wikipedia.org/wiki/Passphrase#Passphrases_selection) to choose a good password phrase.
