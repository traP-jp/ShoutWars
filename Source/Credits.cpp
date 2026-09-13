# include "Credits.hpp"

namespace
{
	constexpr StringView CopyrightKey = U"著作権";
	constexpr StringView SourceKey = U"出典";
	constexpr StringView LicenseKey = U"ライセンス";
	constexpr StringView LicenseFileKey = U"ライセンス文書";

	[[nodiscard]]
	LicenseInfo ToLicenseInfo(const INI& ini, const INISection& section, const FilePathView baseDirectory)
	{
		const String& title = section.section;

		if (title.isEmpty())
		{
			throw Error{ U"Found keys that belong to no work. Write them under a [work name] section." };
		}

		if (not ini.hasValue(title, CopyrightKey))
		{
			throw Error{ U"[{}] is missing '{}'."_fmt(title, CopyrightKey) };
		}

		const bool hasLicenseFile = ini.hasValue(title, LicenseFileKey);

		if (ini.hasValue(title, LicenseKey) == hasLicenseFile)
		{
			throw Error{ U"[{}] must have exactly one of '{}' and '{}'."_fmt(title, LicenseKey, LicenseFileKey) };
		}

		// LicenseManager は text をエスケープせずに HTML へ埋め込むため、こちらでエスケープする。
		String text;

		if (ini.hasValue(title, SourceKey))
		{
			text += (ini.getValue(title, SourceKey).xml_escaped() + U'\n');
		}

		if (hasLicenseFile)
		{
			const String& licenseFile = ini.getValue(title, LicenseFileKey);

			TextReader reader{ Resource(baseDirectory + licenseFile), TextEncoding::UTF8_NO_BOM };

			if (not reader)
			{
				throw Error{ U"Cannot read '{}' specified in [{}]."_fmt(licenseFile, title) };
			}

			text += reader.readAll().xml_escaped();
		}
		else
		{
			text += ini.getValue(title, LicenseKey).xml_escaped();
		}

		return LicenseInfo{
			.title = title,
			.copyright = ini.getValue(title, CopyrightKey),
			.text = text,
		};
	}
}

namespace Credits
{
	Array<LicenseInfo> Load(const FilePathView path)
	{
		const INI ini{ Resource(path), TextEncoding::UTF8_NO_BOM };

		if (not ini)
		{
			throw Error{ U"Cannot read the credits file '{}'. Work names or key names may be duplicated."_fmt(path) };
		}

		// FileSystem::ParentPath は絶対パスを返しリソースパスを壊すため、文字列として切り出す。
		const size_t separator = path.lastIndexOf(U'/');
		const FilePathView baseDirectory = ((separator == StringView::npos) ? FilePathView{} : path.substr(0, (separator + 1)));

		Array<LicenseInfo> licenses;

		for (const auto& section : ini.sections())
		{
			licenses << ToLicenseInfo(ini, section, baseDirectory);
		}

		return licenses;
	}
}
