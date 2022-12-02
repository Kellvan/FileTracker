#pragma once

/// A general interface for file trackers.
class IFileTracker
{
public:
	/// The event handler that will be called each time something happens in a directory.
	using EventHandlerFn = std::function<void(const std::string& dir, const std::string& name)>;

	/// A default destructor.
	virtual ~IFileTracker() = default;

	/// Adds the new directory to tracking.
	virtual void track_directory(const std::string& directory, EventHandlerFn handler) = 0;

	/// Removes the directory from tracking.
	virtual void stop_track(const std::string& directory) = 0;

	/// Checks tracking directories. Should be called as often as tracking needed.
	virtual void check() = 0;
};
