# include "GraphicsQuality.hpp"

GraphicsQuality LoadGraphicsQuality(FilePathView configPath) {
	const JSON config = JSON::Load(configPath);
	if (!config.isObject() || !config.contains(U"graphicsQuality")) return GraphicsQuality::High;
	const String quality = config[U"graphicsQuality"].isString() ? config[U"graphicsQuality"].getString() : U"";
	if (quality == U"low") return GraphicsQuality::Low;
	if (quality == U"high") return GraphicsQuality::High;
	throw Error{ U"graphicsQuality in the config file must be \"low\" or \"high\"." };
}
