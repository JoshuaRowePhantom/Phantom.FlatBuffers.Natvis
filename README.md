# Phantom.FlatBuffers.Natvis

Debugging FlatBuffers can be challenging. This tool generates
Visual Studio .natvis files given a binary FlatBuffer schema.
The resulting debugger visualation is much easier to use.

## Running

The ```Phantom.FlatBuffers.Natvis.exe``` tool provides help via the 
```--help``` command line option. The usual way to run it is to provide
the input schema and output .natvis paths:

```
Phantom.FlatBuffers.Natvis.exe --binary-schema MySchema.fbs --output MySchema.natvis
```

## cmake

The ```PhantomFlatBuffersNatvisTargets.cmake``` file
adds a ```phantom_flatbuffers_generate_natvis``` function
to add .natvis generation to an existing ```flatbuffers_generate_headers```
target.

