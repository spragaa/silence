# boost
# nlohmann json

sudo apt-get -y install libpqxx-dev

# redis
sudo apt update
sudo apt install redis-server
sudo apt install libhiredis-dev
git clone https://github.com/sewenew/redis-plus-plus.git
cd redis-plus-plus
mkdir build
cd build
cmake ..
make
sudo make install

sudo apt-get install libgtest-dev
cd /usr/src/gtest
sudo cmake .
sudo make