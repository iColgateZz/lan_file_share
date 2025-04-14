#include "safe_string.h"
#include <stdlib.h>

string add_title(string template, string uri) {
    string listing = snew("Listing of /");
    string title = scats(listing, uri);
    string with_title = sreplace(template, 6, "#TITLE", sgetlen(title), title);
    sfree(listing); 
    sfree(title);
    return with_title;
}

string add_links(string uri, size_t n, string* dir_file) {
    string li = snew("<li><a href=\"/PATH\">PATH</a></li>\n");

    string first_link = snew("/");
    first_link = scat(first_link, sgetlen(uri), uri);
    if (sgetlen(first_link) != 1) {
        first_link = scat(first_link, 1, "/");
    }

    string links = snew("");
    for (size_t i = 0; i < n; i++) {
        string full_link = scats(first_link, dir_file[i]);

        string new_full_path = sreplace(li, 5, "/PATH", sgetlen(full_link), full_link);
        string link = sreplace(new_full_path, 4, "PATH", sgetlen(dir_file[i]), dir_file[i]);

        string tmp = links;
        links = scats(links, link);

        sfree(full_link); sfree(new_full_path); sfree(link); sfree(tmp);
    }
    sfree(li);
    return links;
}
