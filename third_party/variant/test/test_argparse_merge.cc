//=============================================================================
//! \file   main.cc
//! \author Josh Staley <staley@arlut.utexas.edu>
//=============================================================================

//=== Includes ================================================================

#include <Variant/Variant.h>
#include <Variant/ArgParse.h>
#include <Variant/Schema.h>

#include <iostream>

using namespace std;
using namespace libvariant;

//=== Foo =====================================================================

class Foo {

public:
	
	Foo ( Variant opts = Variant::MapType ):
		mOpts ( opts )
	{
		SchemaResult result = SchemaValidate( Foo::GetArgParse().GenerateSchema(), mOpts );
		if ( result.Error() )
			cerr << "# Foo: " << result.PrettyPrintMessage() << endl;
	}
	
	static ArgParse GetArgParse ()
	{
		ArgParse argParse;
		
		// -v --verbose
		argParse.AddOption( "foo/verbosity", 'v', "foo-verbose",
			"Set foo verbosity level"
		).Action( ARGACTION_COUNT ).Default(0);

		argParse.AddOption( "list[0]/foo", 'f', "foo", "FoOOooOooOoo!" );
		
		return argParse;
	}

private:
	
	Variant mOpts;

};

//=== Bar =====================================================================

class Bar {

public:
	
	Bar ( Variant opts = Variant::MapType ):
		mOpts ( opts )
	{
		SchemaResult result = SchemaValidate( Bar::GetArgParse().GenerateSchema(), mOpts );
		if ( result.Error() )
			cerr << "# Bar: " << result.PrettyPrintMessage() << endl;
	}
	
	static ArgParse GetArgParse ()
	{
		ArgParse argParse;
		
		// -v --verbose
		argParse.AddOption( "bar/verbosity", 'v', "bar-verbose",
			"Set bar verbosity level"
		).Action( ARGACTION_COUNT ).Default(0);

		argParse.AddOption( "list[1]/bar", 'b', "bar", "BaaaaAAAaaar!" );
		
		return argParse;
	}

private:
	
	Variant mOpts;

};

//=== Main ====================================================================

int main ( int argc, char *argv[] )
{
	ArgParse argParse( "Test" );
	argParse.ConflictResolution();
	argParse.Merge( Foo::GetArgParse() );
	argParse.Merge( Bar::GetArgParse() );
	Variant opts = argParse.Parse( argc, argv );
	
	Foo foo( opts );
	Bar bar( opts );
	
	cout << "--- Foo ---" << endl;
	cout << Serialize( Foo::GetArgParse().GenerateSchema(), SERIALIZE_JSON, Variant().Set( "pretty", true ) ) << endl;
	
	cout << "--- Bar ---" << endl;
	cout << Serialize( Bar::GetArgParse().GenerateSchema(), SERIALIZE_JSON, Variant().Set( "pretty", true ) ) << endl;
	
	cout << "--- Merged ---" << endl;
	cout << Serialize( argParse.GenerateSchema(), SERIALIZE_JSON, Variant().Set( "pretty", true ) ) << endl;
	return 0;
}

//=============================================================================
