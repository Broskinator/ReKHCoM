/*
 * Movie (FMV) support. The cartridge's decoder (asm/movie_codec.s) is ARM
 * code copied to work RAM and patched at run time; until its C replacement
 * lands here, movies are skipped and the game continues with the next scene.
 */
int PortMovieSupported(void) {
    return 0;
}
