# include "Credits.hpp"

# include <algorithm>

namespace
{
	constexpr StringView CopyrightKey = U"著作権";
	constexpr StringView SourceKey = U"出典";
	constexpr StringView LicenseKey = U"ライセンス";
	constexpr StringView LicenseFileKey = U"ライセンス文書";

	// 収録先はリポジトリのどのファイルが外部素材かを示すためのもので、表示には用いない。
	constexpr StringView LocationKey = U"収録先";

	constexpr StringView KnownKeys[] = {
		CopyrightKey, SourceKey, LicenseKey, LicenseFileKey, LocationKey,
	};

	// LicenseManager は text をエスケープせずに HTML へ埋め込むため、こちらでエスケープする。
	[[nodiscard]]
	String Escaped(const StringView value)
	{
		return String{ value }.xml_escaped();
	}

	[[nodiscard]]
	String ReadLicenseFile(const FilePathView path)
	{
		TextReader reader{ FileOrResource(path), TextEncoding::UTF8_NO_BOM };

		if (not reader)
		{
			throw Error{ U"ライセンス文書 '{}' を読み込めません。"_fmt(path) };
		}

		return reader.readAll();
	}

	void ValidateKeys(const INISection& section)
	{
		for (const auto& key : section.keys)
		{
			if (std::ranges::none_of(KnownKeys, [&](const StringView known) { return (known == key.name); }))
			{
				throw Error{ U"[{}] に知らない項目 '{}' があります。"_fmt(section.section, key.name) };
			}
		}
	}

	[[nodiscard]]
	LicenseInfo ToLicenseInfo(const INI& ini, const INISection& section)
	{
		const String& title = section.section;

		if (title.isEmpty())
		{
			throw Error{ U"どの作品にも属さない項目があります。項目は [作品名] の下に書いてください。" };
		}

		ValidateKeys(section);

		if (not ini.hasValue(title, CopyrightKey))
		{
			throw Error{ U"[{}] に '{}' がありません。"_fmt(title, CopyrightKey) };
		}

		const bool hasLicense = ini.hasValue(title, LicenseKey);
		const bool hasLicenseFile = ini.hasValue(title, LicenseFileKey);

		if (hasLicense == hasLicenseFile)
		{
			throw Error{ U"[{}] には '{}' と '{}' のどちらか一方だけを書いてください。"_fmt(title, LicenseKey, LicenseFileKey) };
		}

		Array<String> texts;

		if (ini.hasValue(title, SourceKey))
		{
			texts << Escaped(ini.getValue(title, SourceKey));
		}

		texts << Escaped(hasLicenseFile
			? ReadLicenseFile(ini.getValue(title, LicenseFileKey))
			: ini.getValue(title, LicenseKey));

		return LicenseInfo{
			.title = title,
			.copyright = ini.getValue(title, CopyrightKey),
			.text = texts.join(U"\n", U"", U""),
		};
	}
}

namespace Credits
{
	Array<LicenseInfo> Load(const FilePathView path)
	{
		const INI ini{ FileOrResource(path), TextEncoding::UTF8_NO_BOM };

		if (not ini)
		{
			throw Error{ U"クレジットファイル '{}' を読み込めません。作品名や項目名が重複している可能性があります。"_fmt(path) };
		}

		Array<LicenseInfo> licenses;

		for (const auto& section : ini.sections())
		{
			licenses << ToLicenseInfo(ini, section);
		}

		return licenses;
	}

	void Register(const FilePathView path)
	{
		for (const auto& license : Load(path))
		{
			LicenseManager::AddLicense(license);
		}
	}
}
