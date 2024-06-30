```markdown
# AuthProject-PUBLIC

A high-performance server application with integrated DDoS protection, encryption, and user management. This project uses ENet for networking, OpenSSL for encryption, and nlohmann/json for JSON parsing.

## Features

- **DDoS Protection:** Protects the server from DDoS attacks by limiting request rates and packet sizes.
- **Encryption:** Secures communication using AES-256 encryption.
- **User Management:** Manages user data, including adding, removing, banning, and searching for users.

## Prerequisites

- **CMake**: Ensure you have CMake installed. You can download it from [here](https://cmake.org/download/).
- **vcpkg**: This project uses vcpkg for dependency management. Install it by following these steps:

  ```sh
  git clone https://github.com/microsoft/vcpkg.git
  cd vcpkg
  ./bootstrap-vcpkg.sh
  ```

## Getting Started

### Dependency Installation

Use `vcpkg` to install the necessary libraries:

```sh
./vcpkg install enet
./vcpkg install openssl
./vcpkg install nlohmann-json
```

### Building the Project

1. **Clone the repository**:

   ```sh
   git clone https://github.com/subugt/AuthProject-PUBLIC.git
   cd AuthProject-PUBLIC
   ```

2. **Create a `build` directory and navigate into it**:

   ```sh
   mkdir build
   cd build
   ```

3. **Configure the project using CMake**:

   ```sh
   cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
   ```

   > Ensure that the `VCPKG_ROOT` environment variable is set to your vcpkg installation path.

4. **Build the project**:

   ```sh
   cmake --build .
   ```

### Running the Server

After building, you can run the server executable located in the `build` directory:

```sh
./MyServer
```

-----

## Code Overview

### DDoS Protection

The `DDoSProtection` class monitors and controls the rate of requests and packet sizes to prevent DDoS attacks. It registers requests from IP addresses and blocks those that exceed a specified rate or packet size.

### Encryption

The `Encryption` class provides methods for encrypting and decrypting messages using AES-256.

### User Management

The `UserManager` class handles operations related to user management, including adding, removing, banning, and searching for users. It also manages maintenance modes and request blocking.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for any improvements or bug fixes.

## License

This project is licensed under the MIT License. See the [https://github.com/subugt/AuthProject-PUBLIC/blob/master/LICENSE.txt](LICENSE) file for details.

## Acknowledgments

- [ENet](https://github.com/lsalzman/enet)
- [OpenSSL](https://github.com/openssl/openssl)
- [nlohmann/json](https://github.com/nlohmann/json)

# TO DO LIST
- [x] Create Users
- [x] Ban Users
- [x] Search users by username
- [x] Extend version etc.
- [x] Full Ddos protection (limits - low level)
- [x] Build Files ( i will generate visual studio and cmake, gcc for easy build on every os  
- [x] Forward INVALID REQUESTS TO GOOGLE 
- [x] ENCRYPTION AND SAVE WITH JSON
- [x] EXTRA SECURITY ( WEB TOKENS )
- [ ] MACOS, LINUX Support ( not tested but it should support at all we are not sure so unchecked )
- [ ] to cease updating the project indefinitely.

---
LEAVE A STAR FOR SUPPORTING ME 😂