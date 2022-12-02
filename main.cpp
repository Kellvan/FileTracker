#include "stdafx.h"

#include "FileTrackerLinux.h"

namespace
{
FileTrackerLinux::EventHandlerFn handler = [](const std::string& dir, const std::string& name)
{
	std::cout << "New file detected in " << dir << ": " << name << std::endl;

	std::ifstream file;
	const auto path = dir + "/" + name;
	file.open(path);

	auto lines_count{0};
	std::string line;
	while (std::getline(file, line))
		++lines_count;

	file.close();

	std::remove(path.c_str());

	std::cout << "Number of lines in the file: " << lines_count << std::endl;
};
}

int main(int count, char** args)
{
	std::vector<std::string> dirs; //{"./test"};
	for (auto i = 1; i < count; ++i)
		dirs.emplace_back(args[i]);

	if (dirs.empty())
	{
		std::cout << "No directories to track." << std::endl;
		return 0;
	}

	try
	{
		auto tracker = FileTrackerLinux::create();

		for (const auto& dir : dirs)
		{
			std::cout << "Attaching: " << dir << std::endl;
			tracker.track_directory(dir, handler);
		}

		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			tracker.check();
		}
	}
	catch (std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}

	return 0;
}
