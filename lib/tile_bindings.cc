#include <nan.h>
#include <v8.h>
#include "SecondaryTilesLib.h"
#include "TileOptions.h"

using v8::Local;
using v8::String;
using v8::Boolean;
using v8::Integer;

SecondaryTiles::TileOptions ToTileOptions(TileOptions* tileOptionsObject) {
	SecondaryTiles::TileOptions tileOptions;

	tileOptions.Square70x70Logo = tileOptionsObject->Square70x70Logo;
	tileOptions.Square150x150Logo = tileOptionsObject->Square150x150Logo;
	tileOptions.Wide310x150Logo = tileOptionsObject->Wide310x150Logo;
	tileOptions.Square310x310Logo = tileOptionsObject->Square310x310Logo;
	tileOptions.BackgroundColor = tileOptionsObject->BackgroundColor != "undefined" ? tileOptionsObject->BackgroundColor : "";
	tileOptions.ForegroundText = (SecondaryTiles::ForegroundText)tileOptionsObject->ForegroundText;
	tileOptions.ShowNameOnSquare150x150Logo = tileOptionsObject->ShowNameOnSquare150x150Logo;
	tileOptions.ShowNameOnWide310x150Logo = tileOptionsObject->ShowNameOnWide310x150Logo;
	tileOptions.ShowNameOnSquare310x310Logo = tileOptionsObject->ShowNameOnSquare310x310Logo;
	tileOptions.RoamingEnabled = tileOptionsObject->RoamingEnabled;
	tileOptions.InitialBadgeCount = tileOptionsObject->InitialBadgeCount;

	return tileOptions;
}

void RequestCreateOrUpdateInternal(Nan::NAN_METHOD_ARGS_TYPE info, bool update) {
	bool callWithOptions;
	int argLengthOffset = 0;

	if (update)
	{
		argLengthOffset = 1;
	}

	if (info.Length() == 7 - argLengthOffset
		&& info[1 - argLengthOffset]->IsString()
		&& info[2 - argLengthOffset]->IsString()
		&& info[3 - argLengthOffset]->IsString()
		&& info[4 - argLengthOffset]->IsString()
		&& info[5 - argLengthOffset]->IsBoolean()
		&& info[6 - argLengthOffset]->IsBoolean())
	{
		callWithOptions = false;
	}
	else if (info.Length() == 6 - argLengthOffset
		&& info[1 - argLengthOffset]->IsString()
		&& info[2 - argLengthOffset]->IsString()
		&& info[3 - argLengthOffset]->IsString()
		&& info[4 - argLengthOffset]->IsInt32()
		&& info[5 - argLengthOffset]->IsObject())
	{
		callWithOptions = true;
	}
	else
	{
		Nan::ThrowTypeError("Invalid arguments, expected arguments are: hwnd [HWND], tileId [string], displayName [string], arguments [string], squareLogo150x150Uri [string], showNameOnSquare150x150Logo [bool], roamingEnabled [bool]\r\n or hwnd [HWND], tileId [string], displayName [string], arguments [string], tileSize [int], tileOptions [TileOptions]");
		return;
	}

	HWND hWnd;

	if (!update)
	{
		auto bufferObj = Nan::To<v8::Object>(info[0]).ToLocalChecked();
		unsigned char *bufferData = (unsigned char *)node::Buffer::Data(bufferObj);
		unsigned long handle = *reinterpret_cast<unsigned long *>(bufferData);
		hWnd = (HWND)handle;
	}

	String::Utf8Value p1(info[1 - argLengthOffset]->ToString());
	std::string tileId = std::string(*p1);

	Local<String> p2 = Nan::To<String>(info[2 - argLengthOffset]).ToLocalChecked();
	String::Value displayName(p2);

	String::Utf8Value p3(info[3 - argLengthOffset]->ToString());
	std::string arguments = std::string(*p3);


	if (!callWithOptions)
	{
		String::Utf8Value p4(info[4 - argLengthOffset]->ToString());
		std::string squareLogo150x150Uri = std::string(*p4);

		bool showNameOnSquare150x150Logo = info[5 - argLengthOffset]->BooleanValue();
		bool roamingEnabled = info[6 - argLengthOffset]->BooleanValue();

		if (!update)
		{
			SecondaryTiles::RequestCreate(hWnd, tileId, (PCWSTR)* displayName, arguments, squareLogo150x150Uri, showNameOnSquare150x150Logo, roamingEnabled);
		}
		else
		{
			SecondaryTiles::RequestUpdate(tileId, (PCWSTR)* displayName, arguments, squareLogo150x150Uri, showNameOnSquare150x150Logo, roamingEnabled);
		}

		info.GetReturnValue().SetNull();
	}
	else
	{
		int32_t sizeValue = info[4 - argLengthOffset]->Int32Value();

		SecondaryTiles::TileSize desiredSize = (SecondaryTiles::TileSize)sizeValue;
		Nan::MaybeLocal<v8::Object> maybe = Nan::To<v8::Object>(info[5 - argLengthOffset]);
		auto tileOptionsObject = Nan::ObjectWrap::Unwrap<TileOptions>(maybe.ToLocalChecked());

		SecondaryTiles::TileOptions tileOptions = ToTileOptions(tileOptionsObject);

		if (!update)
		{
			SecondaryTiles::RequestCreate(hWnd, tileId, (PCWSTR)* displayName, arguments, desiredSize, tileOptions);
		}
		else
		{
			SecondaryTiles::RequestUpdate(tileId, (PCWSTR)* displayName, arguments, desiredSize, tileOptions);
		}

		info.GetReturnValue().SetNull();
	}
}

NAN_METHOD(RequestCreate) {
	RequestCreateOrUpdateInternal(info, false);
}

NAN_METHOD(RequestUpdate) {
	RequestCreateOrUpdateInternal(info, true);
}

NAN_METHOD(Exists) {

	if (info.Length() < 1 || !info[0]->IsString())
	{
		Nan::ThrowTypeError("Invalid arguments, expected arguments are: tileId [string]");
		return;
	}

	auto local = Nan::To<String>(info[0]).ToLocalChecked();
	auto tileId = std::string(*Nan::Utf8String(local));

	auto result = SecondaryTiles::Exists(tileId);
	auto message = Nan::New<Boolean>(result);

	info.GetReturnValue().Set(message);
}

NAN_METHOD(RequestDelete) {

	if (info.Length() < 1 || !info[0]->IsString())
	{
		Nan::ThrowTypeError("Invalid arguments, expected arguments are: tileId [string]");
		return;
	}

	auto local = Nan::To<String>(info[0]).ToLocalChecked();
	auto tileId = std::string(*Nan::Utf8String(local));

	SecondaryTiles::RequestDelete(tileId);
	info.GetReturnValue().SetNull();
}

NAN_METHOD(Notify) {

	if (info.Length() < 1 || !info[0]->IsString() || !info[1]->IsString())
	{
		Nan::ThrowTypeError("Invalid arguments, expected arguments are: tileId [string], contentXml [string]");
		return;
	}

	auto p0 = Nan::To<v8::String>(info[0]).ToLocalChecked();
	auto tileId = std::string(*Nan::Utf8String(p0));

	auto p1 = Nan::To<String>(info[1]).ToLocalChecked();
	String::Value contentXml(p1);
	
	SecondaryTiles::Notify(tileId, (PCWSTR)* contentXml);
	info.GetReturnValue().SetNull();
}

NAN_METHOD(BadgeNotify) {

	if (info.Length() < 1 || !info[0]->IsString() || !info[1]->IsString())
	{
		Nan::ThrowTypeError("Invalid arguments, expected arguments are: tileId [string], badgeXml [string]");
		return;
	}

	auto p0 = Nan::To<v8::String>(info[0]).ToLocalChecked();
	auto tileId = std::string(*Nan::Utf8String(p0));

	auto p1 = Nan::To<v8::String>(info[1]).ToLocalChecked();
	auto contentXml = std::string(*Nan::Utf8String(p1));

	SecondaryTiles::BadgeNotify(tileId, contentXml);
	info.GetReturnValue().SetNull();
}

NAN_METHOD(ClearNotification) {

	if (info.Length() < 1 || !info[0]->IsString())
	{
		Nan::ThrowTypeError("Invalid argument, expected arguments is: tileId [string]");
		return;
	}

	auto p0 = Nan::To<v8::String>(info[0]).ToLocalChecked();
	auto tileId = std::string(*Nan::Utf8String(p0));

	SecondaryTiles::ClearNotification(tileId);
	info.GetReturnValue().SetNull();
}

NAN_METHOD(ClearBadge) {

	if (info.Length() < 1 || !info[0]->IsString())
	{
		Nan::ThrowTypeError("Invalid argument, expected arguments is: tileId [string]");
		return;
	}

	auto p0 = Nan::To<v8::String>(info[0]).ToLocalChecked();
	auto tileId = std::string(*Nan::Utf8String(p0));

	SecondaryTiles::ClearBadge(tileId);
	info.GetReturnValue().SetNull();
}

NAN_MODULE_INIT(Initialize) {
	NAN_EXPORT(target, RequestCreate);
	NAN_EXPORT(target, RequestUpdate);
	NAN_EXPORT(target, RequestDelete);
	NAN_EXPORT(target, Exists);
	NAN_EXPORT(target, Notify);
	NAN_EXPORT(target, BadgeNotify);
	NAN_EXPORT(target, ClearNotification);
	NAN_EXPORT(target, ClearBadge);
	TileOptions::Init(target);
}

#if NODE_MAJOR_VERSION >= 10
NAN_MODULE_WORKER_ENABLED(tile_bindings, Initialize)
#else
NODE_MODULE(tile_bindings, Initialize)
#endif
