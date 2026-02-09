create extension if not exists unaccent;
create extension if not exists pg_trgm;

create table if not exists stash (
  id bigserial primary key,
  physical_label text,
  category text not null,
  sub_category text not null,
  contents text not null
);
