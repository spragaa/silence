#include <string>

class User {
public:

	User(const std::string& nickname);

	int         get_id() const noexcept;
	std::string get_nickname() const noexcept;

private:

	std::string nickname;
	static int id_counter;
	int user_id;
};
