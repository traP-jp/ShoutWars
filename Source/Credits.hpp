# pragma once
# include <Siv3D.hpp>

namespace Credits
{
	/// @brief クレジットファイルを読み込み、ライセンス情報の一覧を返します。
	/// @param path クレジットファイルのパス
	/// @return ファイルに書かれた順のライセンス情報
	/// @throw Error 読み込みに失敗した場合、または記述に不備がある場合
	[[nodiscard]]
	Array<LicenseInfo> Load(FilePathView path);
}
